#ifndef MEASUREMENTS_H
#define MEASUREMENTS_H

#include "../game_files/Board.h"

namespace Measurements
{
    // A class to be instantiated for one game and then have it's values collected, then die.
    class GameMeasurement{
    public:
        GameMeasurement();

        Board::Board* active_board;

        virtual std::vector<double> get_values()=0;
        virtual void update()=0;
    };

    // A class that can be held by Experiments and used to repeatedly get new measurement classes for each new game
    class MeasurementConstructor{
    public:
        MeasurementConstructor();

        std::string name;
        std::string description;

        virtual std::vector<std::string> get_value_keys()=0;
        virtual GameMeasurement* construct_measure(Board::Board& active_board)=0;
    };

    // A measurement to return only win/lose status of games;
    class WinLose: public GameMeasurement{
    int reward=-1;
    public:
        WinLose(Board::Board& _active_board);

        std::vector<double> get_values();
        void update();
    };

    class WinLoseConstructor: public MeasurementConstructor{
    public:
        WinLoseConstructor();

        std::vector<std::string> get_value_keys();
        GameMeasurement* construct_measure(Board::Board& active_board);
    };

    // To track the status of board attributes that can make an agent lose:
    //      - how many outbreaks there are at the end of the game
    //      - how many cubes of each disease are on the board
    //      - how many cards are left in the player deck
    class LoseStatus: public GameMeasurement{
    public:
        LoseStatus(Board::Board& _active_board);

        std::vector<double> get_values();
        void update();
    };

    class LoseStatusConstructor: public MeasurementConstructor{
    public:
        LoseStatusConstructor();

        std::vector<std::string> get_value_keys();
        GameMeasurement* construct_measure(Board::Board& active_board);
    };

    // A measurement to track the minimum, maximum, average std of branching factor, as well as depth.
    class GameTreeSize: public GameMeasurement{
    std::vector<int> branching_factors={};
    public:
        GameTreeSize(Board::Board& _active_board);

        std::vector<double> get_values();
        void update();
    };

    class GameTreeSizeConstructor: public MeasurementConstructor{
    public:
        GameTreeSizeConstructor();

        std::vector<std::string> get_value_keys();
        GameMeasurement* construct_measure(Board::Board& active_board);
    };

    // A measurement of when each event card first appeared, and when it was used
    class EventCardUse: public GameMeasurement{
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
        void update();
    };

    class EventCardUseConstructor: public MeasurementConstructor{
    public:
        EventCardUseConstructor();

        std::vector<std::string> get_value_keys();
        GameMeasurement* construct_measure(Board::Board& active_board);
    };

    // To track how many diseases were cured in the game. Trackers indicate turn on which each was cured, or -1 if never cured.
    class CuredDisease: public GameMeasurement{
        int BlueCured = -1;
        int YellowCured = -1;
        int BlackCured = -1;
        int RedCured = -1;

        int steps=0;
    public:
        CuredDisease(Board::Board& _active_board);

        std::vector<double> get_values();
        void update();
    };

    class CuredDiseaseConstructor: public MeasurementConstructor{
    public:
        CuredDiseaseConstructor();

        std::vector<std::string> get_value_keys();
        GameMeasurement* construct_measure(Board::Board& active_board);
    };

    // To track how many epidemics are drawn in the game
    class EpidemicsDrawn: public GameMeasurement{
    public:
        EpidemicsDrawn(Board::Board& _active_board);

        std::vector<double> get_values();
        void update();
    };

    class EpidemicsDrawnConstructor: public MeasurementConstructor{
    public:
        EpidemicsDrawnConstructor();

        std::vector<std::string> get_value_keys();
        GameMeasurement* construct_measure(Board::Board& active_board);
    };

    // To track how many epidemics are drawn in the game
    class ResearchStations: public GameMeasurement{
    public:
        ResearchStations(Board::Board& _active_board);

        std::vector<double> get_values();
        void update();
    };

    class ResearchStationsConstructor: public MeasurementConstructor{
    public:
        ResearchStationsConstructor();

        std::vector<std::string> get_value_keys();
        GameMeasurement* construct_measure(Board::Board& active_board);
    };

}

#endif