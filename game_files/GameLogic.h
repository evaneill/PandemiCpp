#ifndef GAMELOGIC_H
#define GAMELOGIC_H

#include <vector>

#include "Board.h"

#include "Actions.h"
#include "StochasticActions.h"

#include "SanityCheck.h"

#include "../agents/Heuristics.h"

#include "../experimental_tools/Scenarios.h"

namespace GameLogic
{
    // This class is built to be the sole entity through which an agent interacts with the game
    class Game{
        // Pointer to a board
        Board::Board* active_board = nullptr;

        // "Vanilla" move constructors

        Actions::MoveConstructor MoveCon;
        Actions::DirectFlightConstructor DirectFlightCon;
        Actions::CharterFlightConstructor CharterFlightCon;
        Actions::ShuttleFlightConstructor ShuttleFlightCon;
        Actions::OperationsExpertFlightConstructor OperationsExpertFlightCon;
        Actions::BuildConstructor BuildCon;
        Actions::TreatConstructor TreatCon;
        Actions::CureConstructor CureCon;
        Actions::GiveConstructor GiveCon;
        Actions::TakeConstructor TakeCon;
        Actions::DoNothingConstructor DoNothingCon;

        // Event constructors

        Actions::AirliftConstructor AirliftCon;
        Actions::GovernmentGrantConstructor GovernmentGrantCon;
        Actions::QuietNightConstructor QuietNightCon;

        // For algorithmic ease of access, to hold all player actions (including event cards)
        const std::array<Actions::ActionConstructor*,13> PlayerConstructorList;

        // ForcedDiscardAction (not voluntary, and not part of num_actions consideration from player turn perspective)
        
        Actions::ForcedDiscardConstructor ForcedDiscardCon;

        // Stochastic actions for use by the Game Logic _only_ (*not* constructors)
        // Like the above, these have legal() guards! v useful

        StochasticActions::StochasticActionConstructor StochasticCon;
    public:
        Game(bool verbose=false);
        ~Game(){
            if(active_board){
                delete active_board;
            }
        };

        // reset the active_board in-place
        void reset_board(Board::Board* new_board);
        bool hasBoard();

        // Pull an action from a uniform distribution over ALL legal actions
        Actions::Action* get_random_action_uniform(bool verbose = false);
        Actions::Action* get_random_action_uniform(Board::Board& game_board,bool verbose=false);
        
        // Pull an action group from uniform selection over action "types", then uniform selection over arguments
        Actions::Action* get_random_action_bygroup(bool verbose = false);
        Actions::Action* get_random_action_bygroup(Board::Board& game_board,bool verbose=false);

        // Return a vector of all legal actions
        std::vector<Actions::Action*> list_actions(bool verbose = false);
        // Return a vector of all legal actions for a given board
        std::vector<Actions::Action*> list_actions(Board::Board& game_board,bool verbose=false);

        // Apply an action given by an agent to the attached active_board
        void applyAction(Actions::Action* action);

        // Return the total number of player actions available
        int n_available_actions(bool verbose = false);
        // Return total number of actions for a given board
        int n_available_actions(Board::Board& game_board,bool verbose = false);

        // ===== Functional necessities =====

        // To advance the non-player elements of the game
        void nonplayer_actions(bool verbose = false);
        // Advance the non-player elements in a given board
        void nonplayer_actions(Board::Board& game_board,bool verbose=false);
        // generate a stochastic action (utility for search and nonplayer_actions)
        Actions::Action* get_stochastic_action(Board::Board& game_board);

        // Check for game status (true if win/loss/broken)
        // sanity check argument to include whether or not to go through all SanityCheck::CheckBoard() checks before determining terminal state.
        bool is_terminal(bool sanity_check=false,bool verbose=false);
        // Check for terminal status in a given game board
        bool is_terminal(Board::Board& game_board,bool sanity_check=false,bool verbose=false);

        // Check if the state of a game board is stochastic
        bool is_stochastic(Board::Board& game_board);

        // return a vector of reasons that the game is terminal, for diagnostics.
        std::vector<std::string> terminal_reasons(); // 

        // game reward. Should return NULL if called on a non-terminal state.
        // Worth it to note that NULL + int is still an int, but can produce a runtime warning
        int reward();
        // reward of a given board
        int reward(Board::Board& game_board);

        // Get the reward from rolling out a game, using a given heuristic to evaluate the end-state
        // By default just return the 0/1 loss/win value
        double rollout(Board::Board* game_board,Heuristics::Heuristic heuristic = Heuristics::PureGameReward);
        double rollout(Board::Board& game_board,Heuristics::Heuristic heuristic = Heuristics::PureGameReward);

        // Board pointer for affixing to actions as necessary
        Board::Board* get_board_ptr();
        // Return a copy of the board (so agents can play with a copy)
        Board::Board board_copy();
    };
}

#endif