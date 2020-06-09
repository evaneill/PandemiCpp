#include "../game_files/Debug.h"

#include "../agents/Agents.h"

#include "Experiments.h"

void Experiments::RunExperiment(Experiments::Experiment* exp){
    // Write the header file including start timestamp (REMOVE any file that existed before)
    // Will need to append the finish time later
    exp -> write_header();

    // ===== End of writing header =====

    // Start  maintaining output file in memory, then dump at the end of the experiment
    
    // Write the header line with log_headers, starting with "Game" in the leftmost column
    std::string output_str;
    // Reserve ~2x the amount of space it needs (a double will be represented by 10 chars w/ comma after, including period)
    output_str.reserve((*exp).n_games*(*exp).measureCons.size()*20);

    output_str+= "Game";
    for(std::string key: (*exp).log_headers){
        output_str+= ","+key;
    }
    output_str+="\r\n";

    // run all the experiments
    DEBUG_MSG("[Experiments::RunExperiment()] Beginning Game loop of " << (*exp).n_games << " games on " << (*exp).experiment_name << "..."<< std::endl);
    
    // Declare run-time stuff
    std::vector<Measurements::GameMeasurement*> game_measures={};
    GameLogic::Game* the_game;
    Board::Board* game_board;
    Agents::BaseAgent* the_agent;

    int games_played=0;
    while(games_played<(*exp).n_games){
        if(games_played % 5000 ==0){
            DEBUG_MSG("[Experiments::RunExperiment()] has ran " << games_played << " games " << std::endl);
        }

        // Write the game number in the first position
        output_str+= std::to_string(games_played+1);

        // Instantiate a new board using the given scenario
        game_board = exp -> get_board();

        // Instantiate game logic pointed at the board
        the_game = new GameLogic::Game(*game_board);

        // Instantiate an agent pointed at the game logic
        the_agent = exp -> get_agent(the_game);

        game_measures = exp -> get_game_measures(game_board);
        
        while(!the_game -> is_terminal(true,false)){
            // First resolve any necessary non-player transisitions (card draws, etc)
            the_game -> nonplayer_actions();
            
            // If these transitions haven't made the game terminal, then have the agent choose a transition
            if(!the_game -> is_terminal(true,false)){
                // Update each measurement
                for(Measurements::GameMeasurement* meas: game_measures){
                    meas -> update();
                }
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
    };

    DEBUG_MSG("[[Experiments::RunExperiment()]] ...successfully executed " + std::to_string(games_played) + " games with the "<< (*exp).experiment_name << " experiment." << std::endl);

    // Write the entire contents of the experiments all at once into a csv (REMOVE any file that existed before)
    exp -> write_experiment(output_str);

    // Append the end time to the header before finishing
    // Thanks stackoverflow, again
    time_t start_time;
    struct tm * timeinfo;
    char buffer[80];

    time (&start_time);
    timeinfo = localtime(&start_time);

    strftime(buffer,sizeof(buffer),"%d-%m-%Y %H:%M:%S",timeinfo);
    std::string str(buffer);

    exp -> append_header("End Time: "+str+"\r\n");
}