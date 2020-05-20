#ifndef STOCHASTICACTIONS_H
#define STOCHASTICACTIONS_H

#include "Actions.h"
#include "Board.h"
#include "Decks.h"

#include <array>

// This namespace holds all of the non-player transitions inherent to the game
// Some of these represent the stochasticity inherent to the non-player moves, but at other times are deterministic but just not controlled by the player
namespace StochasticActions
{
    // I feel like there's probably a better way to show the game logic that this is a stochastic action rather than "plain" action
    class StochasticAction: public Actions::Action{
    public:
        StochasticAction(Board::Board& _active_board): Action(_active_board) {};
    };

    class PlayerDeckDrawAction: public StochasticAction{
        std::string card_drawn;
    public:
        PlayerDeckDrawAction(Board::Board& _active_board);
        // return the action necessary for the player to take next. Potentially only "Do Nothing"
        // will only act on the action IF it's not a "DONOTHING" action, i.e. forced discard or play quiet night
        void execute();
        std::string repr();
        bool legal();
    };

    // Performs the whole infect step. Will stop if game is lost after setting game status to lost
    class InfectDrawAction: public StochasticAction{
        std::string card_drawn;

        // Used to indicate whether or not the Quarantine Specialist was found
        bool QuarantineSpecialistBlocked=false;

        // Used to help describe the outcome of the action (# outbreaks added, # blocked)
        std::array<int,2> outbreak_track;
    public:
        InfectDrawAction(Board::Board& _active_board);
        void execute();
        std::string repr();
        bool legal();
    };

    class StochasticActionConstructor: public Actions::ActionConstructor{
    public:
        StochasticActionConstructor(Board::Board& _active_board);

        // Unlike other constructors, there's no notion of delimiting all actions or counting them (though hypothetically this is possible)
        // We do still want the constructor to give either an InfectDraw or PlayerDraw action
        Actions::Action get_action();

        // Whether or not it's time to apply stochasticity.
        bool legal();
    };
}

#endif