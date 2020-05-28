#include "../game_files/GameLogic.h"

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

        // Scenario name & description (for header file)
        Scenarios::Scenario* scenario;

        // Experiment name (for header file)
        std::string experiment_name; // E.g. "UCTAgentWinScenario"
        std::string description; // E.g. "For evaluating success of agents in trying to..."

        // number of games that aget
        int n_games=0;

        virtual void run()=0;

        // Functional methods
        virtual void write_header()=0;
        virtual void append_header(std::string extras)=0;
        virtual void write_experiment(std::string data)=0;
    };

    class UniformRandomAgentGameExperiment: public Experiment{
    public:
        UniformRandomAgentGameExperiment();

        void run();

        void write_header();
        void append_header(std::string extras);
        void write_experiment(std::string data);
    };
}