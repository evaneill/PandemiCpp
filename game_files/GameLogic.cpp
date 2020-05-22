#include <iostream>

#include "GameLogic.h"
#include "Board.h"
#include "SanityCheck.h"
#include "StochasticActions.h"
#include "Actions.h"

GameLogic::Game::Game(std::vector<int> roles, int difficulty,bool verbose): 
    active_board(roles,difficulty),

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
            std::cout << "Instantiating Game Logic. Setting up the board..." << std::endl;
        }
        active_board.setup();
        if(verbose){
            std::cout << "... success! Board set up" << std::endl;
        }
}

void GameLogic::Game::nonplayer_actions(bool verbose){
    // Designed to go until there there's either a required discard, OR non-player board transitions are complete
    // Right now this SKIPS any use of event cards during draw phase!
    while(StochasticCon.legal() && !ForcedDiscardCon.legal()){
        Actions::Action* next_action = StochasticCon.get_action();
        if(verbose){
            std::cout << active_board.active_player().role.name << ": ";
        }
        next_action -> execute();
        if(verbose){
            std::cout << next_action -> repr() << std::endl;
        }
    }
}

Actions::Action* GameLogic::Game::get_random_action_uniform(bool verbose){
    // Goal is to get a UNIFORM distribution over all legal actions
    // Will _try_ to speed this up over just generating all actions by:
    //      counting player actions first
    //      taking a uniform int in [0,sum of total actions-1]
    //      going through the same action options in order until this falls within one of the ranges
    
    // Like with list_actions, gotta do the execution of any necessary game logic before listing out actions
    nonplayer_actions(verbose);

    if(ForcedDiscardCon.legal()){
        // ALWAYS return a choice of discard actions if it's possible.
        // Includes possible plays of event cards
        return ForcedDiscardCon.random_action();
    }

    // n_available_actions) includes nonzero additions where the group is legal
    int randomized = rand() % n_available_actions();

    // Under this scheme actions should appear uniformly
    for(Actions::ActionConstructor* con_ptr: PlayerConstructorList){
        if(randomized < (con_ptr -> n_actions())){
            return (con_ptr -> random_action());
        }else{
            randomized -= (con_ptr -> n_actions());
        }
    }
    active_board.broken()=true;
    active_board.broken_reasons().push_back("get_random_action_uniform got to end of execution without choosing an action");
}

Actions::Action* GameLogic::Game::get_random_action_bygroup(bool verbose){
    // Goal is to choose FIRST uniformly over _types_ of actions
    // Then choose uniformly among legal actions of that type

    // Have to execute game logic to determine possible actions
    nonplayer_actions(verbose);

    if(ForcedDiscardCon.legal()){
        // ALWAYS return a choice of discard actions if it's possible.
        // Includes possible plays of event cards
        return ForcedDiscardCon.random_action();
    }

    std::vector<Actions::ActionConstructor*> legal_groups;
    for(int con=0;con<PlayerConstructorList.size();con++){
        if(PlayerConstructorList[con] -> legal()){
            legal_groups.push_back(PlayerConstructorList[con]);
        }
    }
    int randomized = rand() % legal_groups.size();

    return legal_groups[randomized] -> random_action();
}

int GameLogic::Game::n_available_actions(bool verbose){
    nonplayer_actions(verbose);
    int total_actions=0;
    for(Actions::ActionConstructor* con_ptr: PlayerConstructorList){
        total_actions+=(con_ptr -> n_actions());
    }

    return  total_actions;
}

std::vector<Actions::Action*> GameLogic::Game::list_actions(bool verbose){
    // This represents a lazy execution of stochastic actions - waits until the next time any action is asked for before resolving any stochasticity/discard
    nonplayer_actions(verbose);

    if(ForcedDiscardCon.legal()){
        // ALWAYS return a choice of discard actions if it's possible.
        // Includes possible plays of event cards
        return ForcedDiscardCon.all_actions();
    }

    // If the logic has reached this point, then we're just letting the player (agent) play. Return action options.
    std::vector<Actions::Action*> full_out;

    for(Actions::ActionConstructor* con_ptr: PlayerConstructorList){
        if(con_ptr -> legal()){
            std::vector<Actions::Action*> these_actions = (con_ptr -> all_actions());
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
    return active_board.is_terminal();
}

int GameLogic::Game::reward(){
    if(active_board.has_lost()){
        return 0;
    } else if(active_board.has_won()){
        return 1;
    } else {
        active_board.broken()=true;
        active_board.broken_reasons().push_back("Asking for reward when win/lose aren't true");
        return -10000000;// NULL gets converted to 0 anyway; this should make it obvious something broke... hoepfully....
    }
}