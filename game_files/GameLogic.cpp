#include <iostream>

#include "GameLogic.h"
#include "Board.h"
#include "SanityCheck.h"
#include "StochasticActions.h"
#include "Actions.h"
#include "Debug.h"

#include "../agents/Heuristics.h"

GameLogic::Game::Game(bool verbose): 
    // Initiate constructors
    MoveCon(),
    DirectFlightCon(),
    CharterFlightCon(),
    ShuttleFlightCon(),
    OperationsExpertFlightCon(),
    BuildCon(),
    TreatCon(),
    CureCon(),
    GiveCon(),
    TakeCon(),

    DoNothingCon(), // (will be treated a little differently than other player actions)

    // Do the same with the forced discard constructor
    ForcedDiscardCon(),

    // Do the same with event card action constructors
    AirliftCon(),
    GovernmentGrantCon(),
    QuietNightCon(),
    
    // Instantiate the list of references to the constructors
    // (purely for algorithmic nicety)
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
            &AirliftCon,
            &GovernmentGrantCon,
            &QuietNightCon}),

    // Instantiate the stochastic action constructor
    StochasticCon()
    {
        if(verbose){
            DEBUG_MSG("[Game::Game(...)] Instantiating Game Logic..." << std::endl);
        }
}

void GameLogic::Game::reset_board(Board::Board* new_board){
    if(active_board){
        // if active board is already assigned, free up the memory
        delete active_board;
    }
    active_board = new_board;
}

bool GameLogic::Game::hasBoard(){
    // return true if active_board isn't nullptr
    // (not a *great* way to check whether the board is well defined)
    // (but in existing implementation is hopefully sufficient)
    return active_board!=nullptr;
}

Actions::Action* GameLogic::Game::get_stochastic_action(Board::Board& game_board){
    return StochasticCon.get_action(game_board);
}

void GameLogic::Game::nonplayer_actions(bool verbose){
    // Designed to go until there there's either a required discard, OR non-player board transitions are complete, OR something breaks or game is lost
    // Right now this SKIPS any use of event cards during draw phase!
    nonplayer_actions(*active_board,verbose);
}


void GameLogic::Game::nonplayer_actions(Board::Board& game_board,bool verbose){
    // Designed to go until there there's either a required discard, OR non-player board transitions are complete, OR something breaks or game is lost
    // Right now this SKIPS any use of event cards during draw phase!

    // Stochastic check _includes_ check for terminal status of game (stochastic transitions can't be legal if the game has ended)
    while(is_stochastic(game_board)){
        // Get the next required action
        Actions::Action* next_action = get_stochastic_action(game_board);
        
        // Track statuses as they went into execute(), where active player name and quiet night status might change
        bool was_quiet_night = game_board.quiet_night_status();
        std::string player_name = game_board.active_player().role.name;

        next_action -> execute(game_board);

        if(verbose && (!was_quiet_night || game_board.get_turn_action()==4 || (game_board.get_turn_action()==5 && game_board.get_infect_cards_drawn()==0))){
            DEBUG_MSG("[Game::nonplayer_actions()] " << player_name << ": " << next_action -> repr() << std::endl);
        }

        delete next_action;
    }
}

Actions::Action* GameLogic::Game::get_random_action_uniform(bool verbose){
    return get_random_action_uniform(*active_board);
}

Actions::Action* GameLogic::Game::get_random_action_uniform(Board::Board& game_board, bool verbose){
    // Goal is to get a UNIFORM distribution over all legal actions
    // Will _try_ to speed this up over just generating all actions by:
    //      counting player actions first
    //      taking a uniform int in [0,sum of total actions-1]
    //      going through the same action options in order until this falls within one of the ranges

    if(verbose){
        Players::Player& active_player = game_board.active_player();
        DEBUG_MSG("[Game::get_random_action_uniform()] "<< active_player.role.name << " is in " << Map::CITY_NAME(active_player.get_position()) <<  " and has hand: ");
        for(int& card: active_player.hand){
            DEBUG_MSG(Decks::CARD_NAME(card) << "; ");
        }
        for(int& card: active_player.event_cards){
            DEBUG_MSG(Decks::CARD_NAME(card) << "; ");
        }
        DEBUG_MSG(" (color count BLUE: "<<game_board.active_player().get_color_count()[Map::BLUE]<< ", YELLOW: " << game_board.active_player().get_color_count()[Map::YELLOW] << ", BLACK: " << game_board.active_player().get_color_count()[Map::BLACK] << ", RED: " << game_board.active_player().get_color_count()[Map::RED] << ")");
        DEBUG_MSG(std::endl);
    }
    if(ForcedDiscardCon.legal(game_board)){
        // ALWAYS return a choice of discard actions if it's possible.
        // Includes possible plays of event cards
        return ForcedDiscardCon.random_action(game_board);
    }

    // For hard rollouts - always return cure in the case that the board is "not that bad"
    if(CureCon.legal(game_board)){
        // If no disease has >=18 cubes on the board
        if(game_board.disease_sum(Map::BLUE)<18 && game_board.disease_sum(Map::YELLOW)<18 && game_board.disease_sum(Map::BLACK)<18 && game_board.disease_sum(Map::RED)<18){
            // And theres <=3 outbreaks
            if(game_board.get_outbreak_count()<=3){
                // Then ALWAYS cure when it's possible
                if(verbose){
                    DEBUG_MSG("[Game::get_random_action_uniform()] Returning Cure since it's available and the board doesn't need a lot of attention (hard rollout criterion)");
                }
                return CureCon.random_action(game_board);
            }
        }
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
            DEBUG_MSG(" which has " << con_ptr -> n_actions(game_board) << " possible actions.");
        }
        if(randomized <= (con_ptr -> n_actions(game_board))){
            return (con_ptr -> random_action(game_board));
        }else{
            randomized -= (con_ptr -> n_actions(game_board));
            if(verbose){
                DEBUG_MSG(" randomized number reduced to " << randomized << ".");
            }
        }
    }
    // If you can get through all the "potentially relevant" actions and still have 1 left over, then it was meant to be that we'll do nothing instead
    if(randomized<=DoNothingCon.n_actions(game_board)){
        return DoNothingCon.random_action(game_board);
    }
    // If there's still stuff left over then there's a problem...
    if(verbose){
        DEBUG_MSG(std::endl << "[Game::get_random_action_uniform()] considered ALL actions for random choice but algorithm failed!");
    }
    game_board.broken()=true;
    game_board.broken_reasons().push_back("[Game::get_random_action_uniform()] get_random_action_uniform got to end of execution without choosing an action");
    return nullptr;
}

Actions::Action* GameLogic::Game::get_random_action_bygroup(bool verbose){
    return get_random_action_bygroup(*active_board,verbose);
}

Actions::Action* GameLogic::Game::get_random_action_bygroup(Board::Board& game_board, bool verbose){
    // Goal is to choose FIRST uniformly over _types_ of actions
    // Then choose uniformly among legal actions of that type

    if(ForcedDiscardCon.legal(game_board)){
        // ALWAYS return a choice of discard actions if it's possible.
        // Includes possible plays of event cards
        return ForcedDiscardCon.random_action(game_board);
    }

    // For hard rollouts - always return cure in the case that the board is "not that bad"
    if(CureCon.legal(game_board)){
        // If no disease has >=18 cubes on the board
        if(game_board.disease_sum(Map::BLUE)<18 && game_board.disease_sum(Map::YELLOW)<18 && game_board.disease_sum(Map::BLACK)<18 && game_board.disease_sum(Map::RED)<18){
            // And theres <=3 outbreaks
            if(game_board.get_outbreak_count()<=3){
                // Then ALWAYS cure when it's possible
                if(verbose){
                    DEBUG_MSG("[Game::get_random_action_bygroup()] Returning Cure since it's available and the board doesn't need a lot of attention (hard rollout criterion)");
                }
                return CureCon.random_action(game_board);
            }
        }
    }
    
    if(verbose){
        DEBUG_MSG("[Game::get_random_action_bygroup()] " << game_board.active_player().role.name << " is in " << Map::CITY_NAME(game_board.active_player().get_position()) <<  " and has hand: ");
        for(int& card: game_board.active_player().hand){
            DEBUG_MSG(Decks::CARD_NAME(card) << "; ");
        }
        for(int& card: game_board.active_player().event_cards){
            DEBUG_MSG(Decks::CARD_NAME(card) << "; ");
        }
        DEBUG_MSG(" (color count BLUE: "<<game_board.active_player().get_color_count()[Map::BLUE]<< ", YELLOW: " << game_board.active_player().get_color_count()[Map::YELLOW] << ", BLACK: " << game_board.active_player().get_color_count()[Map::BLACK] << ", RED: " << game_board.active_player().get_color_count()[Map::RED] << ")");
        DEBUG_MSG(std::endl);
    }

    int max_n_actions =PlayerConstructorList.size();
    if(!TreatCon.legal(game_board) && !CureCon.legal(game_board)){
        // If neither treat nor cure is an option then it's possible that game logic will consider do nothing as an action
        if(DoNothingCon.legal(game_board)){
            max_n_actions++;
        }
    }

    // Generate a random action constructor index
    int randomized = rand() % max_n_actions;
    while(true){
        if(verbose){
            DEBUG_MSG("[get_random_action_bygroup()] Inside the action generation loop! Trying random number " << randomized << " (which is ");
        }
        if(randomized==PlayerConstructorList.size()){
            // if we landed on the DoNothingConstructor, which is outside of the PlayerConstructorList, choose that
            if(verbose){
                DEBUG_MSG(DoNothingCon.get_movetype() << ")" <<std::endl);
            }
            return DoNothingCon.random_action(game_board);
        } else if(PlayerConstructorList[randomized] -> legal(game_board)){
            // otherwise, if the constructor we landed on is legal, do that
            if(verbose){
                DEBUG_MSG(PlayerConstructorList[randomized] -> get_movetype() << ", and is legal!)" <<std::endl);
            }
            return PlayerConstructorList[randomized] -> random_action(game_board);
        } else {
            // otherwise, if the constructor we landed on is illegal, choose a new random number
            if(verbose){
                DEBUG_MSG(PlayerConstructorList[randomized] -> get_movetype() << ", and isn't legal...)" <<std::endl);
            }
            randomized = rand() % max_n_actions;
        }
    }
}

int GameLogic::Game::n_available_actions(bool verbose){
    return n_available_actions(*active_board,verbose);
}

int GameLogic::Game::n_available_actions(Board::Board& game_board, bool verbose){
    if(ForcedDiscardCon.legal(game_board)){
        return ForcedDiscardCon.n_actions(game_board);
    }
    int total_actions=0;
    for(Actions::ActionConstructor* con_ptr: PlayerConstructorList){
        total_actions+=(con_ptr -> n_actions(game_board));
    }
    if(!TreatCon.legal(game_board) && !CureCon.legal(game_board)){
        if(DoNothingCon.legal(game_board)){
            total_actions+=DoNothingCon.n_actions(game_board);
        }
    }
    return  total_actions;
}

std::vector<Actions::Action*> GameLogic::Game::list_actions(bool verbose){
    return list_actions(*active_board,verbose);
}

std::vector<Actions::Action*> GameLogic::Game::list_actions(Board::Board& game_board,bool verbose){
    if(ForcedDiscardCon.legal(game_board)){
        // ALWAYS return a choice of discard actions if it's possible.
        // Includes possible plays of event cards
        return ForcedDiscardCon.all_actions(game_board);
    }

    // If the logic has reached this point, then we're just letting the player (agent) play. Return action options.
    std::vector<Actions::Action*> full_out;

    if(verbose){
        Players::Player& active_player = game_board.active_player();
        DEBUG_MSG("[Game::list_actions()] "<< active_player.role.name << " is in " << Map::CITY_NAME(active_player.get_position()) <<  " and has hand: ");
        for(int& card: active_player.hand){
            DEBUG_MSG(Decks::CARD_NAME(card) << "; ");
        }
        for(int& card: active_player.event_cards){
            DEBUG_MSG(Decks::CARD_NAME(card) << "; ");
        }
        DEBUG_MSG(std::endl);
    }
    // Only put DoNothing in action list if you can't cure or treat
    // (And if so then push it deepest down in the action list)
    if(verbose){
        DEBUG_MSG(std::endl << "[Game::list_actions()] considering movetype " << DoNothingCon.get_movetype() << "...");
    }
    if(!TreatCon.legal(game_board) && !CureCon.legal(game_board)){
        if(verbose){
            DEBUG_MSG("it's legal! There are " << DoNothingCon.n_actions(game_board) << " possible actions.");
        }
        if(DoNothingCon.legal(game_board)){
            full_out.push_back(DoNothingCon.random_action(game_board));
        }
    }

    for(Actions::ActionConstructor* con_ptr: PlayerConstructorList){
        if(verbose){
            DEBUG_MSG(std::endl << "[Game::list_actions()] considering movetype " << con_ptr -> get_movetype() << "...");
        }
        if(con_ptr -> legal(game_board)){
            std::vector<Actions::Action*> these_actions = (con_ptr -> all_actions(game_board));
            if(verbose){
                DEBUG_MSG("it's legal! There are " << these_actions.size() << " possible actions.");
            }
            full_out.insert(full_out.end(),these_actions.begin(),these_actions.end());
        }
    }
    
    return full_out;
}

void GameLogic::Game::applyAction(Actions::Action* action){
    // TODO: Guard this! Break the board with illegal actions! Force errors! Prevent catastrophe!
    active_board -> reset_last_action();
    action -> execute(*active_board);
}

bool GameLogic::Game::is_stochastic(Board::Board& game_board){
    // It's a stochastic state if (1) it's a stochastic phase of the game, (2) Forced discard isn't required, and (3) game hasn't ended
    return StochasticCon.legal(game_board) && !ForcedDiscardCon.legal(game_board) && !is_terminal(game_board);
}

bool GameLogic::Game::is_terminal(bool sanity_check,bool verbose){
    // sanity_check does some logical checking of the game state to enforce brokenness
    // costs some computation, which is why it's optional.
    return is_terminal(*active_board,sanity_check, verbose);
}

bool GameLogic::Game::is_terminal(Board::Board& game_board, bool sanity_check,bool verbose){
    // sanity_check does some logical checking of the game state to enforce brokenness
    // costs some computation, which is why it's optional.
    if(sanity_check){
        SanityCheck::CheckBoard(game_board,verbose);
    }

    // Have the board update any win/lose/broken status (this is an intentional redundancy)
    game_board.updatestatus();

    bool broken = game_board.broken();
    if(broken && verbose){
        for(std::string reason: game_board.broken_reasons()){
            DEBUG_MSG(std::endl << "[Game::is_terminal()] Game broke! One reason: " << reason);
        }
    }
    bool won = game_board.has_won();
    bool lost = game_board.has_lost();
    if(lost && verbose){
        DEBUG_MSG(std::endl << "[Game::is_terminal()] LOST! because " <<  game_board.get_lost_reason() << std::endl);
    }
    return won || lost || broken;
}

int GameLogic::Game::reward(){
    return reward(*active_board);
}

int GameLogic::Game::reward(Board::Board& game_board){
    if(game_board.has_lost()){
        return 0;
    } else if(game_board.has_won()){
        return 1;
    } else {
        game_board.broken()=true;
        game_board.broken_reasons().push_back("[Game::reward()] Asking for reward when win/lose aren't true");
        return -10000000;// NULL gets converted to 0 anyway; this should make it obvious something broke... hoepfully....
    }
}

double GameLogic::Game::rollout(Board::Board* game_board,Heuristics::Heuristic heuristic){
    return rollout(*game_board,heuristic);
}

double GameLogic::Game::rollout(Board::Board& game_board,Heuristics::Heuristic heuristic){
    // I've made the executive decision to always use the random_action_bygroup() method!
    // This is like the original game loop in `Experiment.cpp` BUT:
    //      * NO SANITY CHECK
    //      * NO MEASUREMENTS
    //      (both of these changes allow for it to go a good deal faster)
    while(!is_terminal(game_board)){
        nonplayer_actions(game_board);
        if(!is_terminal(game_board)){
            Actions::Action* random_action = get_random_action_bygroup(game_board);
            random_action -> execute(game_board);
            delete random_action;
        }
    }
    // once the game has ended, use the heuristic to evaluate it
    return heuristic(game_board);
}

double GameLogic::Game::epsgreedy_heuristic_rollout(Board::Board& game_board, double epsilon, Heuristics::Heuristic selection_heuristic, Heuristics::Heuristic eval_heuristic){
    while(!is_terminal(game_board)){
        nonplayer_actions(game_board);
        if(!is_terminal(game_board)){
            // get all available action options
            std::vector<Actions::Action*> all_actions = list_actions(game_board);

            // instantiate tracking objects for best action and its heuristic score
            double best_score = -1;
            Actions::Action* best_action = nullptr;

            // For each action...
            for(Actions::Action* act : all_actions){
                // Explicit copy current state copy of the current board state
                Board::Board board_copy = Board::Board(game_board);

                // Apply this action to the board
                act -> execute(board_copy);

                // Use the heuristic to evaluate the result, including a check for terminality
                double state_score;
                if(!board_copy.is_terminal()){
                    // use the selection heuristic to evaluate successor states
                    double state_score = selection_heuristic(board_copy);
                } else {
                    // If terminal, get the reward from the game logic
                    double state_score = reward(board_copy);
                }
                if(state_score>best_score){
                    // If this score is better than the existing max, then reset the "best" score and action.
                    best_score = state_score;
                    best_action = act;
                }
            }

            // If a random uniform number <= epsilon, then...
            if((float) rand()/ (float) RAND_MAX <= epsilon){
                // Choose a non-max action uniformly at random
                int random_index = rand() % all_actions.size();
                while(all_actions[random_index]==best_action && all_actions.size()>1){
                    // make sure we didn't just choose action already defined as best
                    // Make sure that there's at least one other non-"best" action to choose from to avoid infinite loops
                    random_index = rand() % all_actions.size();
                }
                
                // redefine the best action
                best_action = all_actions[random_index];
            }

            // use the "best action" to advance the board
            best_action -> execute(game_board);

            // Lastly delete everything to avoid a fat memory leak
            for(Actions::Action* act : all_actions){
                delete act;
            }
        }
    }
    return eval_heuristic(game_board);
}

std::vector<std::string> GameLogic::Game::terminal_reasons(){
    std::vector<std::string> reasons;
    if((*active_board).has_won()){
        reasons.push_back("Players won!");
    }
    if((*active_board).has_lost()){
        reasons.push_back((*active_board).get_lost_reason());
    }
    if((*active_board).broken()){
        for(std::string reason: (*active_board).broken_reasons()){
            reasons.push_back(reason);
        }
    }
    return reasons;
}

Board::Board GameLogic::Game::board_copy(){
    return *active_board;
};

Board::Board* GameLogic::Game::get_board_ptr(){
    return active_board;
};