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
    class PlayerDeckDrawAction: public Actions::Action{
        std::string card_drawn;
    public:
        PlayerDeckDrawAction();
        
        void execute(Board::Board& game_board);

        std::string repr();
        bool legal(Board::Board& board);
    };

    // Performs the whole infect step. Will stop if game is lost after setting game status to lost
    class InfectDeckDrawAction: public Actions::Action{
        std::string card_drawn;

        // Used to indicate whether or not the Quarantine Specialist was found
        bool QuarantineSpecialistBlocked=false;

        // Used to help describe the outcome of the action (# outbreaks added, # blocked)
        std::array<int,2> outbreak_track;
    public:
        InfectDeckDrawAction();

        void execute(Board::Board& game_board);

        std::string repr();
        bool legal(Board::Board& board);
    };

    class StochasticActionConstructor{
    public:
        StochasticActionConstructor();

        InfectDeckDrawAction infect_draw;
        PlayerDeckDrawAction player_draw;

        // Unlike other constructors, there's no notion of delimiting all actions or counting them (though hypothetically this is possible)
        // We do still want the constructor to give either an InfectDraw or PlayerDraw action to GameLogic
        Actions::Action* get_action(Board::Board& game_board);

        // Whether or not it's time to apply stochasticity.
        bool legal(Board::Board& board);
    };
}

#endif