#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>

#include "../agents/UniformRandomAgent.h"

#include "../experimental_tools/Experiments.h"
#include "../experimental_tools/Scenarios.h"
#include "../experimental_tools/Measurements.h"

Experiments::UniformRandomAgentCanWinExperiment::UniformRandomAgentCanWinExperiment(){
    // Hard-code a description for this experiment
    experiment_name = "UniformRandomAgentCanWinExperiment";
    description = "Test a random agent with a team of Quarantine Specialist, Medic, Scientist, Researcher with 4-epidemic difficulty in contrived scenario where its easy to win";

    fileheader = "UniformRandomAgentCanWinExperiment";// .header ,.csv
    
    // Use the scenario to setup some variables
    scenario = new Scenarios::CanWinScenario();

    agent_name = "Uniform Random Agent";

    // Define measurements on the active board
    // As I write this, these are all the possible measurements
    measureCons = {
        new Measurements::WinLoseConstructor(),
        new Measurements::LoseStatusConstructor(),
        new Measurements::EventCardUseConstructor(),
        new Measurements::EpidemicsDrawnConstructor(),
        new Measurements::CuredDiseaseConstructor(),
        new Measurements::ResearchStationsConstructor(),
        new Measurements::GameTreeSizeConstructor(),
        new Measurements::TimeTakenConstructor()
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

    // Play 10000 games
    n_games=10000;
}

void Experiments::UniformRandomAgentCanWinExperiment::write_header(){
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

void Experiments::UniformRandomAgentCanWinExperiment::append_header(std::string extras){
    std::ofstream header(Experiments::OUTPUT_DIR + fileheader+".header",std::ios::out | std::ios::app);
    header << extras;
    header.close();
}

void Experiments::UniformRandomAgentCanWinExperiment::write_experiment(std::string data){
    std::ofstream logfile(Experiments::OUTPUT_DIR + fileheader + ".csv",std::ios::out | std::ios::trunc);
    logfile << data;
    logfile.close();
}

Board::Board* Experiments::UniformRandomAgentCanWinExperiment::get_board(){
    return &(*scenario).make_board({},4); // This scenario needs the arguments but doesn't use them
}

Agents::BaseAgent* Experiments::UniformRandomAgentCanWinExperiment::get_agent(GameLogic::Game* game){
    return new Agents::UniformRandomAgent(*game);
}

std::vector<Measurements::GameMeasurement*> Experiments::UniformRandomAgentCanWinExperiment::get_game_measures(Board::Board* game){
    std::vector<Measurements::GameMeasurement*> game_measures = {};

    for(Measurements::MeasurementConstructor* con: measureCons){
        game_measures.push_back(con -> construct_measure(*game));
    }
    return game_measures;
}

int main(){
    Experiments::Experiment* experiment = new Experiments::UniformRandomAgentCanWinExperiment();
    
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