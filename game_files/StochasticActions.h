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
        Decks::PlayerCard card_drawn;
        // extra info added to repr() output
        std::string strrep="";
    public:
        PlayerCardDrawAction(Decks::PlayerCard card);
        
        void execute(Board::Board& game_board);

        std::string repr();
        bool legal(Board::Board& board);
    };

    // Executes the effect of drawing an epidemic card
    class EpidemicDrawAction: public Actions::Action{
        // which city is drawn from bottom of infect deck
        Decks::InfectCard card_drawn;
        // The epidemic card drawn (for updating the player deck)
        Decks::PlayerCard epidemic_card;
        // extra info added to repr() output
        std::string strrep="";
    public:
        EpidemicDrawAction(Decks::PlayerCard _epidemic_card,Decks::InfectCard card);

        void execute(Board::Board& game_board);

        std::string repr();
    };

    // Executes effects of drawing an infect card. Will stop if game is lost after setting game status to lost
    class InfectDeckDrawAction: public Actions::Action{
        // infect card drawn
        Decks::InfectCard card_drawn;
        // extra info that can be added to repr()
        std::string strrep="";

        // Used to indicate whether or not the Quarantine Specialist was found
        bool QuarantineSpecialistBlocked=false;

        // Used to help describe the outcome of the action (# outbreaks added, # blocked)
        std::array<int,2> outbreak_track;
    public:
        InfectDeckDrawAction(Decks::InfectCard card);

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