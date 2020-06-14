#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>

#include "../agents/KSampleNaiveUCTAgent.h"

#include "../experimental_tools/Experiments.h"
#include "../experimental_tools/Scenarios.h"
#include "../experimental_tools/Measurements.h"

Experiments::SingleSampleNaiveUCTAgentExperiment::SingleSampleNaiveUCTAgentExperiment(){
    // Hard-code a description for this experiment
    experiment_name = "SingleSampleNaiveUCTAgentExperiment";
    description = "Test a single-determinization UCT agent that uses only win/loss (0/1) reward from rollouts to update node scores, and 10000 simulations/step";

    fileheader = "SingleSampleNaiveUCTAgentExperiment";// .header ,.csv
    
    // Use the scenario to setup some variables
    scenario = new Scenarios::VanillaGameScenario();

    agent_name = "Single-Sample Naive UCT Agent";

    // Define measurements on the active board
    // As I write this, these are all the possible measurements
    measureCons = {
        new Measurements::WinLoseConstructor(),
        new Measurements::LoseStatusConstructor(),
        new Measurements::EventCardUseConstructor(),
        new Measurements::ActionCountConstructor(),
        new Measurements::EpidemicsDrawnConstructor(),
        new Measurements::CuredDiseaseConstructor(),
        new Measurements::ResearchStationsConstructor(),
        new Measurements::GameTreeSizeConstructor(),
        new Measurements::TimeTakenConstructor(),
    };

    // Define the log headers by pushing all the keys together in they same order they'll be evaluated
    // (unless the compiler has other ideas)
    for(Measurements::MeasurementConstructor* con: measureCons){
        for(std::string key: con-> get_value_keys()){
            // I'm HOPING this pushes it back in the order they're supposed to be in
            log_headers.push_back(key);
        }
    }
    log_headers.push_back("BrokeReasons"); // Always track any reasons the board broke

    // Play 1 game
    n_games=1;
}

void Experiments::SingleSampleNaiveUCTAgentExperiment::write_header(){
    std::ofstream header(Experiments::OUTPUT_DIR + fileheader+".header",std::ios::out | std::ios::trunc);

    header << "Experiment Name: " << experiment_name << std::endl;
    header << "Experiment Description: " << description << std::endl << std::endl;
    
    header << "Scenario Name: " << (*scenario).name << std::endl;
    header << "Scenario Description: " << (*scenario).description << std::endl << std::endl;
    
    header << "Agent Name: " << agent_name << std::endl;
    header << "==========================================" << std::endl;
    header << "=========== Measurements Taken ===========" << std::endl<< std::endl;

    for(Measurements::MeasurementConstructor* con: measureCons){
        header << "Measurement Name: " << (*con).name << std::endl;
        header << "Measurement Description: " << (*con).description << std::endl << std::endl;
    }

    // Thanks stackoverflow, again
    time_t start_time;
    struct tm * timeinfo;
    char buffer[80];

    time (&start_time);
    timeinfo = localtime(&start_time);

    strftime(buffer,sizeof(buffer),"%d-%m-%Y %H:%M:%S",timeinfo);
    std::string str(buffer);

    header << "Start Time: " << str << std::endl;
    // End of stackoverflow copypasta


    header.close();
}

void Experiments::SingleSampleNaiveUCTAgentExperiment::reset_board(Board::Board* game_board){
    scenario -> reset_board(game_board);
}

void Experiments::SingleSampleNaiveUCTAgentExperiment::append_header(std::string extras){
    std::ofstream header(Experiments::OUTPUT_DIR + fileheader+".header",std::ios::out | std::ios::app);
    header << extras;
    header.close();
}

void Experiments::SingleSampleNaiveUCTAgentExperiment::write_experiment(std::string data){
    std::ofstream logfile(Experiments::OUTPUT_DIR + fileheader + ".csv",std::ios::out | std::ios::trunc);
    logfile << data;
    logfile.close();
}

Board::Board* Experiments::SingleSampleNaiveUCTAgentExperiment::get_board(){
    return scenario -> make_board({1,2,3},4);
}

Agents::BaseAgent* Experiments::SingleSampleNaiveUCTAgentExperiment::get_agent(GameLogic::Game* game){
    // 10000 simulations per step
    // 1 determinization per stochasticity
    return new Agents::KSampleNaiveUCTAgent(*game,10000,1);
}

std::vector<Measurements::GameMeasurement*> Experiments::SingleSampleNaiveUCTAgentExperiment::get_game_measures(Board::Board* game){
    std::vector<Measurements::GameMeasurement*> game_measures = {};

    for(Measurements::MeasurementConstructor* con: measureCons){
    game_measures.push_back(con -> construct_measure(*game));
    }
    return game_measures;
}

int main(){
    Experiments::Experiment* experiment = new Experiments::SingleSampleNaiveUCTAgentExperiment();
    
    // ===== Seed rand() =====
    // ===== Thank you stackoverflow =====
    // https://stackoverflow.com/questions/20201141/same-random-numbers-generated-every-time-in-c
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    /* using nano-seconds instead of seconds */
    srand((time_t)ts.tv_nsec);
    // ===== End of stack overflow copypasta ===== 

    Experiments::RunExperiment(experiment);
}