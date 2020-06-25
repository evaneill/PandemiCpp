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
        virtual ~Experiment(){}
        
        // Simple agent name
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

        // For giving objects to the RunExperiment() method and resetting them
        virtual Board::Board* get_board()=0;
        virtual Agents::BaseAgent* get_agent(GameLogic::Game* game)=0;

        // Retrieve measurements on the game board
        virtual std::vector<Measurements::GameMeasurement*> get_game_measures(Board::Board* board)=0;

        // Use the existing scenario to reset a board in-place
        virtual void reset_board(Board::Board* game_board)=0;

    };

    // A function to take an experiment and run it.
    // This is it's own standalone just because the internal functionality is practically always the same
    void RunExperiment(Experiment* exp,bool log_output=false);

    class UniformRandomAgentGameExperiment: public Experiment{
    public:
        UniformRandomAgentGameExperiment();
        ~UniformRandomAgentGameExperiment(){
            delete scenario;
            for(Measurements::MeasurementConstructor* cons: measureCons){
                delete cons;
            }
            measureCons.clear();
        };

        void write_header();
        void append_header(std::string extras);
        void write_experiment(std::string data);
        
        Board::Board* get_board();
        Agents::BaseAgent* get_agent(GameLogic::Game* game);
        std::vector<Measurements::GameMeasurement*> get_game_measures(Board::Board* board);
        void reset_board(Board::Board* game_board);
    };

    class UniformRandomAgentCanWinExperiment: public Experiment{
    public:
        UniformRandomAgentCanWinExperiment();
        ~UniformRandomAgentCanWinExperiment(){
            delete scenario;
            for(Measurements::MeasurementConstructor* cons: measureCons){
                delete cons;
            }
            measureCons.clear();
        };

        void write_header();
        void append_header(std::string extras);
        void write_experiment(std::string data);
        
        Board::Board* get_board();
        Agents::BaseAgent* get_agent(GameLogic::Game* game);
        std::vector<Measurements::GameMeasurement*> get_game_measures(Board::Board* board);
        void reset_board(Board::Board* game_board);
    };

    class ByGroupRandomAgentGameExperiment: public Experiment{
    public:
        ByGroupRandomAgentGameExperiment();
        ~ByGroupRandomAgentGameExperiment(){
            delete scenario;
            for(Measurements::MeasurementConstructor* cons: measureCons){
                delete cons;
            }
            measureCons.clear();
        };  

        void write_header();
        void append_header(std::string extras);
        void write_experiment(std::string data);
        
        Board::Board* get_board();
        Agents::BaseAgent* get_agent(GameLogic::Game* game);
        std::vector<Measurements::GameMeasurement*> get_game_measures(Board::Board* board);
        void reset_board(Board::Board* game_board);
    };

    class SingleSampleNaiveUCTAgentExperiment: public Experiment {
    public:
        SingleSampleNaiveUCTAgentExperiment();
        ~SingleSampleNaiveUCTAgentExperiment(){
            delete scenario;
            for(Measurements::MeasurementConstructor* cons: measureCons){
                delete cons;
            }
            measureCons.clear();
        };

        void write_header();
        void append_header(std::string extras);
        void write_experiment(std::string data);

        Board::Board* get_board();
        Agents::BaseAgent* get_agent(GameLogic::Game* game);
        std::vector<Measurements::GameMeasurement*> get_game_measures(Board::Board* board);
        void reset_board(Board::Board* game_board);
    };

    class ThreeSampleNaiveUCTAgentExperiment: public Experiment {
    public:
        ThreeSampleNaiveUCTAgentExperiment();
        ~ThreeSampleNaiveUCTAgentExperiment(){
            delete scenario;
            for(Measurements::MeasurementConstructor* cons: measureCons){
                delete cons;
            }
            measureCons.clear();
        };

        void write_header();
        void append_header(std::string extras);
        void write_experiment(std::string data);

        Board::Board* get_board();
        Agents::BaseAgent* get_agent(GameLogic::Game* game);
        std::vector<Measurements::GameMeasurement*> get_game_measures(Board::Board* board);
        void reset_board(Board::Board* game_board);
    };

    class SingleSampleGoalHeuristicUCTAgentExperiment: public Experiment {
    public:
        SingleSampleGoalHeuristicUCTAgentExperiment();
        ~SingleSampleGoalHeuristicUCTAgentExperiment(){
            delete scenario;
            for(Measurements::MeasurementConstructor* cons: measureCons){
                delete cons;
            }
            measureCons.clear();
        };

        void write_header();
        void append_header(std::string extras);
        void write_experiment(std::string data);

        Board::Board* get_board();
        Agents::BaseAgent* get_agent(GameLogic::Game* game);
        std::vector<Measurements::GameMeasurement*> get_game_measures(Board::Board* board);
        void reset_board(Board::Board* game_board);
    };

    class ThreeSampleGoalHeuristicUCTAgentExperiment: public Experiment {
    public:
        ThreeSampleGoalHeuristicUCTAgentExperiment();
        ~ThreeSampleGoalHeuristicUCTAgentExperiment(){
            delete scenario;
            for(Measurements::MeasurementConstructor* cons: measureCons){
                delete cons;
            }
            measureCons.clear();
        };

        void write_header();
        void append_header(std::string extras);
        void write_experiment(std::string data);

        Board::Board* get_board();
        Agents::BaseAgent* get_agent(GameLogic::Game* game);
        std::vector<Measurements::GameMeasurement*> get_game_measures(Board::Board* board);
        void reset_board(Board::Board* game_board);
    };

    class SingleSampleSubGoalHeuristicUCTAgentExperiment: public Experiment {
    public:
        SingleSampleSubGoalHeuristicUCTAgentExperiment();
        ~SingleSampleSubGoalHeuristicUCTAgentExperiment(){
            delete scenario;
            for(Measurements::MeasurementConstructor* cons: measureCons){
                delete cons;
            }
            measureCons.clear();
        };

        void write_header();
        void append_header(std::string extras);
        void write_experiment(std::string data);

        Board::Board* get_board();
        Agents::BaseAgent* get_agent(GameLogic::Game* game);
        std::vector<Measurements::GameMeasurement*> get_game_measures(Board::Board* board);
        void reset_board(Board::Board* game_board);
    };

    class ThreeSampleSubGoalHeuristicUCTAgentExperiment: public Experiment {
    public:
        ThreeSampleSubGoalHeuristicUCTAgentExperiment();
        ~ThreeSampleSubGoalHeuristicUCTAgentExperiment(){
            delete scenario;
            for(Measurements::MeasurementConstructor* cons: measureCons){
                delete cons;
            }
            measureCons.clear();
        };

        void write_header();
        void append_header(std::string extras);
        void write_experiment(std::string data);

        Board::Board* get_board();
        Agents::BaseAgent* get_agent(GameLogic::Game* game);
        std::vector<Measurements::GameMeasurement*> get_game_measures(Board::Board* board);
        void reset_board(Board::Board* game_board);
    };

    class ThreeSampleSubGoalHeuristic50kUCTAgentExperiment: public Experiment {
    public:
        ThreeSampleSubGoalHeuristic50kUCTAgentExperiment();
        ~ThreeSampleSubGoalHeuristic50kUCTAgentExperiment(){
            delete scenario;
            for(Measurements::MeasurementConstructor* cons: measureCons){
                delete cons;
            }
            measureCons.clear();
        };

        void write_header();
        void append_header(std::string extras);
        void write_experiment(std::string data);

        Board::Board* get_board();
        Agents::BaseAgent* get_agent(GameLogic::Game* game);
        std::vector<Measurements::GameMeasurement*> get_game_measures(Board::Board* board);
        void reset_board(Board::Board* game_board);
    };

    class SingleSampleNaive50kUCTAgentExperiment: public Experiment {
    public:
        SingleSampleNaive50kUCTAgentExperiment();
        ~SingleSampleNaive50kUCTAgentExperiment(){
            delete scenario;
            for(Measurements::MeasurementConstructor* cons: measureCons){
                delete cons;
            }
            measureCons.clear();
        };

        void write_header();
        void append_header(std::string extras);
        void write_experiment(std::string data);

        Board::Board* get_board();
        Agents::BaseAgent* get_agent(GameLogic::Game* game);
        std::vector<Measurements::GameMeasurement*> get_game_measures(Board::Board* board);
        void reset_board(Board::Board* game_board);
    };

    class ThreeSampleGoalHeuristic50kUCTAgentExperiment: public Experiment {
    public:
        ThreeSampleGoalHeuristic50kUCTAgentExperiment();
        ~ThreeSampleGoalHeuristic50kUCTAgentExperiment(){
            delete scenario;
            for(Measurements::MeasurementConstructor* cons: measureCons){
                delete cons;
            }
            measureCons.clear();
        };

        void write_header();
        void append_header(std::string extras);
        void write_experiment(std::string data);

        Board::Board* get_board();
        Agents::BaseAgent* get_agent(GameLogic::Game* game);
        std::vector<Measurements::GameMeasurement*> get_game_measures(Board::Board* board);
        void reset_board(Board::Board* game_board);
    };

    class SingleSampleGoalHeuristic50kUCTAgentExperiment: public Experiment {
    public:
        SingleSampleGoalHeuristic50kUCTAgentExperiment();
        ~SingleSampleGoalHeuristic50kUCTAgentExperiment(){
            delete scenario;
            for(Measurements::MeasurementConstructor* cons: measureCons){
                delete cons;
            }
            measureCons.clear();
        };

        void write_header();
        void append_header(std::string extras);
        void write_experiment(std::string data);

        Board::Board* get_board();
        Agents::BaseAgent* get_agent(GameLogic::Game* game);
        std::vector<Measurements::GameMeasurement*> get_game_measures(Board::Board* board);
        void reset_board(Board::Board* game_board);
    };

    class ThreeSampleNaive50kUCTAgentExperiment: public Experiment {
    public:
        ThreeSampleNaive50kUCTAgentExperiment();
        ~ThreeSampleNaive50kUCTAgentExperiment(){
            delete scenario;
            for(Measurements::MeasurementConstructor* cons: measureCons){
                delete cons;
            }
            measureCons.clear();
        };

        void write_header();
        void append_header(std::string extras);
        void write_experiment(std::string data);

        Board::Board* get_board();
        Agents::BaseAgent* get_agent(GameLogic::Game* game);
        std::vector<Measurements::GameMeasurement*> get_game_measures(Board::Board* board);
        void reset_board(Board::Board* game_board);
    };

    class SingleSampleSubGoalHeuristic50kUCTAgentExperiment: public Experiment {
    public:
        SingleSampleSubGoalHeuristic50kUCTAgentExperiment();
        ~SingleSampleSubGoalHeuristic50kUCTAgentExperiment(){
            delete scenario;
            for(Measurements::MeasurementConstructor* cons: measureCons){
                delete cons;
            }
            measureCons.clear();
        };

        void write_header();
        void append_header(std::string extras);
        void write_experiment(std::string data);

        Board::Board* get_board();
        Agents::BaseAgent* get_agent(GameLogic::Game* game);
        std::vector<Measurements::GameMeasurement*> get_game_measures(Board::Board* board);
        void reset_board(Board::Board* game_board);
    };

    class K3_10k_SubGoal_UCTMaxChildExperiment: public Experiment {
    public:
        K3_10k_SubGoal_UCTMaxChildExperiment();
        ~K3_10k_SubGoal_UCTMaxChildExperiment(){
            delete scenario;
            for(Measurements::MeasurementConstructor* cons: measureCons){
                delete cons;
            }
            measureCons.clear();
        };

        void write_header();
        void append_header(std::string extras);
        void write_experiment(std::string data);

        Board::Board* get_board();
        Agents::BaseAgent* get_agent(GameLogic::Game* game);
        std::vector<Measurements::GameMeasurement*> get_game_measures(Board::Board* board);
        void reset_board(Board::Board* game_board);
    };

    class K1_10k_Naive_UCTMaxChildExperiment: public Experiment {
    public:
        K1_10k_Naive_UCTMaxChildExperiment();
        ~K1_10k_Naive_UCTMaxChildExperiment(){
            delete scenario;
            for(Measurements::MeasurementConstructor* cons: measureCons){
                delete cons;
            }
            measureCons.clear();
        };

        void write_header();
        void append_header(std::string extras);
        void write_experiment(std::string data);

        Board::Board* get_board();
        Agents::BaseAgent* get_agent(GameLogic::Game* game);
        std::vector<Measurements::GameMeasurement*> get_game_measures(Board::Board* board);
        void reset_board(Board::Board* game_board);
    };
}