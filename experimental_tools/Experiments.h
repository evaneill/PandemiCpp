#include "../game_files/GameLogic.h"
#include "../game_files/Board.h"

#include "Scenarios.h"
#include "Measurements.h"

#include "../agents/Agents.h"

namespace Experiments
{

    inline const std::string OUTPUT_DIR = "results/";

    class Experiment{
    public:
        Experiment(){};

        std::string agent_name;

        // The measurements tracked over the course of the experiment, given as constructors
        std::vector<Measurements::MeasurementConstructor*> measureCons;

        // file prefix for outputs
        std::string fileheader ="";

        // Output column headers
        std::vector<std::string> log_headers = {};

        // Scenario 
        Scenarios::Scenario* scenario;

        // Experiment name (for header file)
        std::string experiment_name; // E.g. "UniformRandomAgentGameScenario"
        std::string description; // E.g. "For evaluating success of agents in trying to..."

        // number of games that aget
        int n_games=0;

        // Functional methods
        
        // For writing results
        virtual void write_header()=0;
        virtual void append_header(std::string extras)=0;
        virtual void write_experiment(std::string data)=0;

        // For giving objects to the RunExperiment() method
        virtual Board::Board* get_board()=0;
        virtual Agents::BaseAgent* get_agent(GameLogic::Game* game)=0;
        virtual std::vector<Measurements::GameMeasurement*> get_game_measures(Board::Board* board)=0;

    };

    // A function to take an experiment and run it.
    // This is it's own standalone just because the internal functionality is practically always the same
    void RunExperiment(Experiment* exp);

    class UniformRandomAgentGameExperiment: public Experiment{
    public:
        UniformRandomAgentGameExperiment();

        void write_header();
        void append_header(std::string extras);
        void write_experiment(std::string data);
        
        Board::Board* get_board();
        Agents::BaseAgent* get_agent(GameLogic::Game* game);
        std::vector<Measurements::GameMeasurement*> get_game_measures(Board::Board* board);
    };
}