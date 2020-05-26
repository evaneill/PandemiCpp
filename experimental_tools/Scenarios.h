#ifndef SCENARIOS_H
#define SCENARIOS_H

#include "../game_files/Board.h"

#include "../game_files/Decks.h"
#include "../game_files/Players.h"

#include "../game_files/Debug.h"

namespace Scenarios
{
    class Scenario{
    public:
        Scenario(std::string name,std::string desc);

        std::string name;
        std::string description;
        
        virtual Board::Board& make_board(std::vector<int> roles,int _difficulty,bool verbose=false)=0;
    };
    
    // This is the simplest example: Use the existing setup() method and create a "normal" board.
    // Setup occurs according to the rules given in the game.
    class VanillaGameScenario: public Scenario{
    public:
        VanillaGameScenario();

        Board::Board& make_board(std::vector<int> roles,int _difficulty,bool verbose=false);
    };

    class ForcedDiscardScenario: public Scenario{
    public:
        ForcedDiscardScenario();

        Board::Board& make_board(std::vector<int> roles,int _difficulty,bool verbose=false);
    };

    class BusyBoardScenario: public Scenario{
    public:
        BusyBoardScenario();

        Board::Board& make_board(std::vector<int> roles,int _difficulty,bool verbose=false);
    };

    class CanWinScenario: public Scenario{
    public:
        CanWinScenario();

        Board::Board& make_board(std::vector<int> roles,int _difficulty,bool verbose=false);
    };
}

#endif