#include <iostream>

#include "GameLogic.h"
#include "Board.h"
#include "SanityCheck.h"
#include "StochasticActions.h"
#include "Actions.h"
#include "Debug.h"

GameLogic::Game::Game(Board::Board _active_board, bool verbose): 
    active_board(_active_board),

    // Link the active board to each constructor
    MoveCon(active_board),
    DirectFlightCon(active_board),
    CharterFlightCon(active_board),
    ShuttleFlightCon(active_board),
    OperationsExpertFlightCon(active_board),
    BuildCon(active_board),
    TreatCon(active_board),
    CureCon(active_board),
    GiveCon(active_board),
    TakeCon(active_board),
    DoNothingCon(active_board),

    // Do the same with the forced discard constructor
    ForcedDiscardCon(active_board),

    // Do the same with event card action constructors
    AirliftCon(active_board),
    GovernmentGrantCon(active_board),
    QuietNightCon(active_board),
    
    // Instantiate the list of references to the constructors
    PlayerConstructorList({
            &MoveCon,
            &DirectFlightCon,
            &CharterFlightCon,
            &ShuttleFlightCon,
            &OperationsExpertFlightCon,
            &BuildCon,
            &TreatCon,
            &CureCon,
            &GiveCon,
            &TakeCon,
            &DoNothingCon,
            &AirliftCon,
            &GovernmentGrantCon,
            &QuietNightCon}),

    // Instantiate the stochastic action constructor
    StochasticCon(active_board)
    {
        if(verbose){
            DEBUG_MSG("[Game::Game(...)] Instantiating Game Logic..." << std::endl);
        }
}

void GameLogic::Game::nonplayer_actions(bool verbose){
    // Designed to go until there there's either a required discard, OR non-player board transitions are complete, OR something breaks or game is lost
    // Right now this SKIPS any use of event cards during draw phase!
    while(StochasticCon.legal() && !ForcedDiscardCon.legal() && !is_terminal()){
        Actions::Action* next_action = StochasticCon.get_action();
        
        // Track statuses as they went into execute(), where active player name and quiet night status might change
        bool was_quiet_night = active_board.quiet_night_status();
        std::string player_name = active_board.active_player().role.name;

        next_action -> execute();

        if(verbose && (!was_quiet_night || active_board.get_turn_action()==4 || (active_board.get_turn_action()==5 && active_board.get_infect_cards_drawn()==0))){
            DEBUG_MSG("[Game::nonplayer_actions()] " << player_name << ": " << next_action -> repr() << std::endl);
        }
    }
}

Actions::Action* GameLogic::Game::get_random_action_uniform(bool verbose){
    // Goal is to get a UNIFORM distribution over all legal actions
    // Will _try_ to speed this up over just generating all actions by:
    //      counting player actions first
    //      taking a uniform int in [0,sum of total actions-1]
    //      going through the same action options in order until this falls within one of the ranges

    if(verbose){
        Players::Player& active_player = active_board.active_player();
        DEBUG_MSG("[Game::get_random_action_uniform()] "<< active_player.role.name << " is in " << active_player.get_position().name <<  " and has hand: ");
        for(Decks::PlayerCard card: active_player.hand){
            DEBUG_MSG(card.name << "; ");
        }
        for(Decks::PlayerCard card: active_player.event_cards){
            DEBUG_MSG(card.name << "; ");
        }
        DEBUG_MSG(std::endl);
    }
    if(ForcedDiscardCon.legal()){
        // ALWAYS return a choice of discard actions if it's possible.
        // Includes possible plays of event cards
        return ForcedDiscardCon.random_action();
    }

    if(verbose){
        DEBUG_MSG("[Game::get_random_action_uniform()] There are a total of " << n_available_actions() << " available actions according to n_available_actions()");
    }
    // n_available_actions) includes nonzero additions where the group is legal
    int randomized = rand() % n_available_actions(verbose);
    if(verbose){
        DEBUG_MSG(std::endl << "[Game::get_random_action_uniform()] Chose random number: " << randomized << " to choose an action.");
    }
    // Under this scheme actions should appear uniformly
    for(Actions::ActionConstructor* con_ptr: PlayerConstructorList){
        if(verbose){
            DEBUG_MSG(std::endl << "[Game::get_random_action_uniform()] "<< (*con_ptr).get_movetype() << " being considered for random choice,");
            DEBUG_MSG(" which has " << con_ptr -> n_actions() << " possible actions.");
        }
        if(randomized <= (con_ptr -> n_actions())){
            return (con_ptr -> random_action());
        }else{
            randomized -= (con_ptr -> n_actions());
            if(verbose){
                DEBUG_MSG(" randomized number reduced to " << randomized << ".");
            }
        }
    }
    if(verbose){
        DEBUG_MSG(std::endl << "[Game::get_random_action_uniform()] considered ALL actions for random choice but algorithm failed!");
    }
    active_board.broken()=true;
    active_board.broken_reasons().push_back("[Game::get_random_action_uniform()] get_random_action_uniform got to end of execution without choosing an action");
}

Actions::Action* GameLogic::Game::get_random_action_bygroup(bool verbose){
    // Goal is to choose FIRST uniformly over _types_ of actions
    // Then choose uniformly among legal actions of that type

    if(ForcedDiscardCon.legal()){
        // ALWAYS return a choice of discard actions if it's possible.
        // Includes possible plays of event cards
        return ForcedDiscardCon.random_action();
    }

    std::vector<Actions::ActionConstructor*> legal_groups;
    if(verbose){
        DEBUG_MSG("[Game::get_random_action_bygroup()] " << active_board.active_player().role.name << " is in " << active_board.active_player().get_position().name <<  " and has hand: ");
        for(Decks::PlayerCard card: active_board.active_player().hand){
            DEBUG_MSG(card.name << "; ");
        }
        for(Decks::PlayerCard card: active_board.active_player().event_cards){
            DEBUG_MSG(card.name << "; ");
        }
        DEBUG_MSG(std::endl);
    }
    for(int con=0;con<PlayerConstructorList.size();con++){
        if(verbose){
            DEBUG_MSG(std::endl << "[Game::get_random_action_bygroup()] considering " << PlayerConstructorList[con] -> get_movetype() <<"... ");
        }
        if(PlayerConstructorList[con] -> legal()){
            if(verbose){
                DEBUG_MSG(" it's legal!");
            }
            legal_groups.push_back(PlayerConstructorList[con]);
        }
    }

    int randomized = rand() % legal_groups.size();

    return legal_groups[randomized] -> random_action();
}

int GameLogic::Game::n_available_actions(bool verbose){
    int total_actions=0;
    for(Actions::ActionConstructor* con_ptr: PlayerConstructorList){
        total_actions+=(con_ptr -> n_actions());
    }

    return  total_actions;
}

std::vector<Actions::Action*> GameLogic::Game::list_actions(bool verbose){
    if(ForcedDiscardCon.legal()){
        // ALWAYS return a choice of discard actions if it's possible.
        // Includes possible plays of event cards
        return ForcedDiscardCon.all_actions();
    }

    // If the logic has reached this point, then we're just letting the player (agent) play. Return action options.
    std::vector<Actions::Action*> full_out;

    if(verbose){
        Players::Player& active_player = active_board.active_player();
        DEBUG_MSG("[Game::list_actions()] "<< active_player.role.name << " is in " << active_player.get_position().name <<  " and has hand: ");
        for(Decks::PlayerCard card: active_player.hand){
            DEBUG_MSG(card.name << "; ");
        }
        for(Decks::PlayerCard card: active_player.event_cards){
            DEBUG_MSG(card.name << "; ");
        }
        DEBUG_MSG(std::endl);
    }

    for(Actions::ActionConstructor* con_ptr: PlayerConstructorList){
        if(verbose){
            DEBUG_MSG(std::endl << "[Game::list_actions()] considering movetype " << con_ptr -> get_movetype() << "...");
        }
        if(con_ptr -> legal()){
            std::vector<Actions::Action*> these_actions = (con_ptr -> all_actions());
            if(verbose){
                DEBUG_MSG("it's legal! There are " << these_actions.size() << " possible actions.");
            }
            full_out.insert(full_out.end(),these_actions.begin(),these_actions.end());
        }
    }
    return full_out;

}

bool GameLogic::Game::is_terminal(bool sanity_check,bool verbose){
    // sanity_check does some logical checking of the game state to enforce brokenness
    // costs some computation, which is why it's optional.
    if(sanity_check){
        SanityCheck::CheckBoard(active_board,verbose);
    }
    // Have the board update any win/lose/broken status
    active_board.updatestatus();

    bool broken = active_board.broken();
    if(broken && verbose){
        for(std::string reason: active_board.broken_reasons()){
            DEBUG_MSG(std::endl << "[Game::is_terminal()] Game broke! One reason: " << reason);
        }
    }
    bool won = active_board.has_won();
    bool lost = active_board.has_lost();
    if(lost && verbose){
        DEBUG_MSG(std::endl << "[Game::is_terminal()] LOST! because " <<  active_board.get_lost_reason());
    }
    return won || lost || broken;
}

int GameLogic::Game::reward(){
    if(active_board.has_lost()){
        return 0;
    } else if(active_board.has_won()){
        return 1;
    } else {
        active_board.broken()=true;
        active_board.broken_reasons().push_back("[Game::reward()] Asking for reward when win/lose aren't true");
        return -10000000;// NULL gets converted to 0 anyway; this should make it obvious something broke... hoepfully....
    }
}

std::vector<std::string> GameLogic::Game::terminal_reasons(){
    std::vector<std::string> reasons;
    if(active_board.has_won()){
        reasons.push_back("Players won!");
    }
    if(active_board.has_lost()){
        reasons.push_back(active_board.get_lost_reason());
    }
    if(active_board.broken()){
        for(std::string reason: active_board.broken_reasons()){
            reasons.push_back(reason);
        }
    }
    return reasons;
}