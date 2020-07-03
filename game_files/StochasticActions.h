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
    // Executes the effect of drawing a player card
    class PlayerCardDrawAction: public Actions::Action{
        // Which card is drawn
        int card_drawn=-1;
        // extra info added to repr() output
        std::string strrep="";
    public:
        PlayerCardDrawAction(int card);
        
        void execute(Board::Board& game_board);

        std::string repr();
        bool legal(Board::Board& board);
    };

    // Executes the effect of drawing an epidemic card
    class EpidemicDrawAction: public Actions::Action{
        // which city is drawn from bottom of infect deck
        int infect_card_drawn=-1;
        // The epidemic card drawn (for updating the player deck)
        int epidemic_card=-1;
        // extra info added to repr() output
        std::string strrep="";
    public:
        EpidemicDrawAction(int _epidemic_card,int _infect_card_drawn);

        void execute(Board::Board& game_board);

        std::string repr();
    };

    // Utility action used to force the game to lose when the player deck is empty and a card is to be drawn
    class PlayerDeckEmptyAction: public Actions::Action{
    public:
        PlayerDeckEmptyAction();

        void execute(Board::Board& game_board);

        std::string repr();
    };

    // Executes effects of drawing an infect card. Will stop if game is lost after setting game status to lost
    class InfectDeckDrawAction: public Actions::Action{
        // infect card drawn
        int card_drawn=-1;
        // extra info that can be added to repr()
        std::string strrep="";

        // Used to indicate whether or not the Quarantine Specialist was found
        bool QuarantineSpecialistBlocked=false;

        // Used to help describe the outcome of the action (# outbreaks added, # blocked)
        std::array<int,2> outbreak_track;
    public:
        InfectDeckDrawAction(int card);

        void execute(Board::Board& game_board);

        std::string repr();
    };

    class PlayerDeckDrawActionConstructor: public Actions::ActionConstructor{
        const std::string movetype = "PLAYERDRAW"; 
    public:
        PlayerDeckDrawActionConstructor();

        // Get a representation
        std::string get_movetype();
        
        // how many legal actions there are
        int n_actions(Board::Board& game_board); 
        Actions::Action* random_action(Board::Board& game_board); // return a random action of thie movetype with legal arguments
        std::vector<Actions::Action*> all_actions(Board::Board& game_board); // NOT USED FOR STOCHASTIC ACTIONS
        bool legal(Board::Board& game_board); // whether there is any legal use of this type of action
    };

    class InfectDeckDrawActionConstructor: public Actions::ActionConstructor{
        const std::string movetype = "INFECTDRAW"; 
    public:
        InfectDeckDrawActionConstructor();

        // Get a representation
        std::string get_movetype();
        
        // how many legal actions there are
        int n_actions(Board::Board& game_board); 
        Actions::Action* random_action(Board::Board& game_board); // return a random action of thie movetype with legal arguments
        std::vector<Actions::Action*> all_actions(Board::Board& game_board); // NOT USED FOR STOCHASTIC ACTIONS
        bool legal(Board::Board& game_board); // whether there is any legal use of this type of action
    };

    class StochasticActionConstructor{
    public:
        StochasticActionConstructor();

        InfectDeckDrawActionConstructor infect_draw_con;
        PlayerDeckDrawActionConstructor player_draw_con;

        // We do still want the constructor to give either an InfectDraw or PlayerDraw action to GameLogic
        Actions::Action* get_action(Board::Board& game_board);

        // Whether or not it's time to apply stochasticity.
        bool legal(Board::Board& board);
    };
}

#endif