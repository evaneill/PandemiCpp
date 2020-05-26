#ifndef MEASUREMENTS_H
#define MEASUREMENTS_H

#include "../game_files/Board.h"

namespace Measurements
{
    class Measurement{
    public:
        Measurement(Board::Board& _active_board);

        Board::Board* active_board;

        std::string name="";
        std::string description="";

        virtual std::vector<double> get_values()=0;
        virtual std::vector<std::string> get_value_keys()=0;
        virtual void update();
    };

    // A measurement to return only win/lose status of games;
    class WinLose: public Measurement{
    int reward=-1;
    public:
        WinLose(Board::Board& _active_board);

        std::vector<double> get_values();
        std::vector<std::string> get_value_keys();
        void update();
    };

    // To track the status of board attributes that can make an agent lose:
    //      - how many outbreaks there are at the end of the game
    //      - how many cubes of each disease are on the board
    //      - how many cards are left in the player deck
    class LoseStatus: public Measurement{
    public:
        LoseStatus(Board::Board& _active_board);

        std::vector<double> get_values();
        std::vector<std::string> get_value_keys();
        void update();
    };

    // A measurement to track the minimum, maximum, average std of branching factor, as well as depth.
    class GameTreeSize: public Measurement{
    std::vector<int> branching_factors={};
    public:
        GameTreeSize(Board::Board& _active_board);

        std::vector<double> get_values();
        std::vector<std::string> get_value_keys();
        void update();
    };

    // A measurement of when each event card first appeared, and when it was used
    class EventCardUse: public Measurement{
    // For tracking how many game steps have occurred
    int steps=0;

    // -1 indicates it wasn't ever seen (for "...Presence") or it was never used (for "...Use")

    int firstQuietNightPresence=-1;
    int QuietNightUse=-1;
    
    int firstAirliftPresence=-1;
    int AirliftUse=-1;

    int firstGovernmentGrantPresence=-1;
    int GovernmentGrantUse=-1;

    public:
        EventCardUse(Board::Board& _active_board);

        std::vector<double> get_values();
        std::vector<std::string> get_value_keys();
        void update();
    };

    // To track how many diseases were cured in the game
    class CuredDisease: public Measurement{
    public:
        CuredDisease(Board::Board& _active_board);

        std::vector<double> get_values();
        std::vector<std::string> get_value_keys();
        void update();
    };

    // To track how many epidemics are drawn in the game
    class EpidemicsDrawn: public Measurement{
    public:
        EpidemicsDrawn(Board::Board& _active_board);

        std::vector<double> get_values();
        std::vector<std::string> get_value_keys();
        void update();
    };

}

#endif