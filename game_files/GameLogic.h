#ifndef GAMELOGIC_H
#define GAMELOGIC_H

#include <vector>

#include "Board.h"

#include "Actions.h"
#include "StochasticActions.h"

#include "SanityCheck.h"

namespace GameLogic
{
    // This class is built to be the sole entity through which an agent interacts with the game
    class Game{
        // This is NOT a reference or pointer. All constructed actions and action constructors point to this board.
        Board::Board active_board;

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
        const std::array<Actions::ActionConstructor*,14> PlayerConstructorList;

        // ForcedDiscardAction (not voluntary, and not part of num_actions consideration from player turn perspective)
        
        Actions::ForcedDiscardConstructor ForcedDiscardCon;

        // Stochastic actions for use by the Game Logic _only_ (*not* constructors)
        // Like the above, these have legal() guards! v useful

        StochasticActions::StochasticActionConstructor StochasticCon;
    public:
        Game(Board::Board _active_board, bool verbose=false);

        // Pull an action from a uniform distribution over ALL legal actions
        Actions::Action* get_random_action_uniform(bool verbose = false);
        
        // Pull an action group from uniform selection over action "types", then uniform selection over arguments
        Actions::Action* get_random_action_bygroup(bool verbose = false);

        // Return a vector of all legal actions
        std::vector<Actions::Action*> list_actions(bool verbose = false);

        // Return the total number of player actions available
        int n_available_actions(bool verbose = false);

        // ===== Functional necessities =====

        // To advance the non-player elements of the game
        void nonplayer_actions(bool verbose = false);

        // Check for game status (true if win/loss/broken)
        // sanity check argument to include whether or not to go through all SanityCheck::CheckBoard() checks before determining terminal state.
        bool is_terminal(bool sanity_check=false,bool verbose=false);

        // game reward. Should return NULL if called on a non-terminal state.
        // Worth it to note that NULL + int is still an int, but can produce a runtime warning
        int reward();
    };
}

#endif