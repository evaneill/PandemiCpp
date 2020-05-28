#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>

#include "../agents/UniformRandomAgent.h"

#include "Experiments.h"
#include "Scenarios.h"
#include "Measurements.h"

Experiments::UniformRandomAgentGameExperiment::UniformRandomAgentGameExperiment(){
    // Hard-code a description for this experiment
    experiment_name = "UniformRandomAgentExperiment";
    description = "Test a random agent with a team of Medic, Scientist, Researcher with 4-epidemic difficulty. Keep all available measurements";

    fileheader = "UniformRandomAgentExperiment";// .header ,.csv
    
    // Use the scenario to setup some variables
    scenario = new Scenarios::VanillaGameScenario();

    agent_name = "Uniform Random Agent";

    // Define measurements on the active board
    // As I write this, these are all the possible measurements
    measureCons = {
        new Measurements::WinLoseConstructor(),
        new Measurements::LoseStatusConstructor(),
        new Measurements::EventCardUseConstructor(),
        new Measurements::EpidemicsDrawnConstructor(),
        new Measurements::CuredDiseaseConstructor(),
        new Measurements::GameTreeSizeConstructor()
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

    // Play 1000 games
    n_games=1000;
}

void Experiments::UniformRandomAgentGameExperiment::write_header(){
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

void Experiments::UniformRandomAgentGameExperiment::append_header(std::string extras){
    std::ofstream header(Experiments::OUTPUT_DIR + fileheader+".header",std::ios::out | std::ios::app);
    header << extras;
    header.close();
}

void Experiments::UniformRandomAgentGameExperiment::write_experiment(std::string data){
    std::ofstream logfile(Experiments::OUTPUT_DIR + fileheader + ".csv",std::ios::out | std::ios::trunc);
    logfile << data;
    logfile.close();
}

void Experiments::UniformRandomAgentGameExperiment::run(){
    // Write the header file including start timestamp (REMOVE any file that existed before)
    // Will need to append the finish time later
    write_header();

    // ===== End of writing header =====

    // Start  maintaining output file in memory, then dump at the end of the experiment
    
    // Write the header line with log_headers, starting with "Game" in the leftmost column
    std::string output_str;
    // Reserve ~2x the amount of space it needs (a double will be represented by 10 chars w/ comma after, including period)
    output_str.reserve(n_games*measureCons.size()*20);

    output_str+= "Game";
    for(std::string key: log_headers){
        output_str+= ","+key;
    }
    output_str+="\r\n";

    // run all the experiments
    int games_played=0;
    DEBUG_MSG("[UniformRandomAgentGameExperiment::run()] Beginning Game loop of " << n_games << " games..." << std::endl);
    
    // Declare run-time stuff
    std::vector<Measurements::GameMeasurement*> game_measures={};
    GameLogic::Game* the_game;
    Board::Board* game_board;
    Agents::UniformRandomAgent* the_agent;

    while(games_played<n_games){

        // Write the game number in the first position
        output_str+= std::to_string(games_played+1);

        // Instantiate a new board using the given scenario
        game_board = &(*scenario).make_board({1,2,3},4,false);

        // Instantiate game logic pointed at the board
        the_game = new GameLogic::Game(*game_board);

        // Instantiate an agent pointed at the game logic
        the_agent = new Agents::UniformRandomAgent(*the_game);

        // Instantiate objects to measure dimensions of specifically this game
        for(Measurements::MeasurementConstructor* con: measureCons){
            game_measures.push_back(con -> construct_measure(*game_board));
        }

        if(games_played%50==0){
            DEBUG_MSG("[UniformRandomAgentGameExperiment::run()] beginning game " << games_played << std::endl);
        }
        int steps=0;
        while(!the_game -> is_terminal(true,false)){
            // First resolve any necessary non-player transisitions (card draws, etc)
            the_game -> nonplayer_actions();
            
            // If these transitions haven't made the game terminal, then have the agent choose a transition
            if(!the_game -> is_terminal(true,false)){
                // Update each measurement
                for(Measurements::GameMeasurement* meas: game_measures){
                    meas -> update();
                }
                steps++;
                // Have the agent take a step
                the_agent -> take_step();
            }
        } 
        // At the end of the game, write out the resulting measures to the output_str
        for(Measurements::GameMeasurement* meas: game_measures){
            std::vector<double> output_values = meas -> get_values();
            for(double& val: output_values){
                output_str+=","+std::to_string(val);
            }
        }
        output_str+=",";

        // Brokenness might need a new entry-point
        if(the_game ->reward()<0){
            std::vector<std::string> broke_reasons = the_game -> terminal_reasons();
            for(std::string& reason: broke_reasons){
                output_str+=reason+";";
            }
        }
        output_str+="\r\n";

        games_played++;

        // Get rid of & reset objects
        game_measures.clear();

        delete the_agent;
        delete the_game;
        delete game_board;
    }

    DEBUG_MSG("[UniformRandomAgentGameExperiment::run()] ...successfully executed " + std::to_string(games_played) + " games." << std::endl);

    // Write the entire contents of the experiments all at once into a csv (REMOVE any file that existed before)
    write_experiment(output_str);

    DEBUG_MSG("[UniformRandomAgentGameExperiment::run()] beginning of output_str: " << std::endl << output_str.substr(0,1000) << std::endl);

    // Append the end time to the header before finishing
    // Thanks stackoverflow, again
    time_t start_time;
    struct tm * timeinfo;
    char buffer[80];

    time (&start_time);
    timeinfo = localtime(&start_time);

    strftime(buffer,sizeof(buffer),"%d-%m-%Y %H:%M:%S",timeinfo);
    std::string str(buffer);

    append_header("End Time: "+str);
}

int main(){
    Experiments::Experiment* experiment = new Experiments::UniformRandomAgentGameExperiment();
    
    // ===== Seed rand() =====
    // ===== Thank you stackoverflow =====
    // https://stackoverflow.com/questions/20201141/same-random-numbers-generated-every-time-in-c
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    /* using nano-seconds instead of seconds */
    srand((time_t)ts.tv_nsec);
    // ===== End of stack overflow copypasta ===== 

    experiment -> run();
}