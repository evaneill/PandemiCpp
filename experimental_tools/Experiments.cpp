#include "../game_files/Debug.h"

#include "../agents/Agents.h"

#include "Experiments.h"

void Experiments::RunExperiment(Experiments::Experiment* exp,bool log_output){
    // Write the header file including start timestamp (REMOVE any file that existed before)
    // Will need to append the finish time later
    exp -> write_header();

    // ===== End of writing header =====

    // Start  maintaining output file in memory, then dump at the end of the experiment
    
    // Write the header line with log_headers, starting with "Game" in the leftmost column
    std::string output_str;
    // Reserve ~4x the amount of space it needs (a double will be represented by 10 chars w/ comma after, including period)
    //  (Padding is added since it doesn't account for potential agent measurements)
    output_str.reserve((*exp).n_games*(*exp).measureCons.size()*40);

    output_str+= "Game";
    // Log headers inclue "broken reasons" at the end
    for(std::string key: (*exp).log_headers){
        output_str+= ","+key;
    }

    // run all the experiments
    DEBUG_MSG("[Experiments::RunExperiment()] Beginning Game loop of " << (*exp).n_games << " games on " << (*exp).experiment_name << "..."<< std::endl);
    
    // Declare run-time stuff
    std::vector<Measurements::GameMeasurement*> game_measures={};

    // instantiate a game that doesn't have a board yet
    GameLogic::Game* the_game = new GameLogic::Game();
    // instantiate an agent pointed to that game logic
    Agents::BaseAgent* the_agent = exp -> get_agent(the_game);
    Board::Board* game_board = nullptr;

    // Append any agent measures after everything else
    if(the_agent -> measurable){
        for(std::string key: the_agent -> get_keys()){
            output_str+=","+key;
        }
    }
    output_str+="\r\n";

    int games_played=0;

    // By int coercion, log_interval should be floor of this division I think?
    // This is every 5% of games completed (unless <20 n_games, then 1)
    int log_interval = ((int) (*exp).n_games/20) > 0 ? ((int) (*exp).n_games/20) : 1;

    // Enter the experimental loop!
    while(games_played<(*exp).n_games){
        if(log_output){
            if(games_played % log_interval==0){
                DEBUG_MSG("[Experiments::RunExperiment()] has ran " << games_played << " games (~"<< ((int) (100*games_played)/(*exp).n_games) << "% completed)" << std::endl);
            }
        }

        // Write the game number in the first column of the experiment output
        output_str+= std::to_string(games_played+1);

        // If there isn't already a board
        if(!game_board){
            // Instantiate a new board using the given scenario
            game_board = exp -> get_board();
        } else {
            // otherwise use the experiment definition to reset the board using its scenario
            exp -> reset_board(game_board);
        }

        if(!the_game -> hasBoard()){
            // if the game logic isn't assigned a board yet (iteration 0), then attach it to this one
            the_game -> reset_board(game_board);
        }

        // reset/instantiate measures
        if(game_measures.empty()){
            game_measures = exp -> get_game_measures(game_board);
        } else {
            for(Measurements::GameMeasurement* meas: game_measures){
                meas -> reset(*game_board);
            }
        }
        
        // Reset any game-level info held in the agent for play
        the_agent -> reset();

        while(!the_game -> is_terminal(true,false)){
            // First resolve any necessary non-player transisitions (card draws)
            the_game -> nonplayer_actions();
            
            // If these transitions haven't made the game terminal, then have the agent choose a transition (no sanity check here)
            if(!the_game -> is_terminal(false,false)){
                // Update each measurement
                for(Measurements::GameMeasurement* meas: game_measures){
                    meas -> update();
                }
                // Have the agent take a step
                //      (Agents with measurements will update them during this step)
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
        // If the agent has measurements to contribute, then add them one at a time
        if(the_agent -> measurable){
            std::vector<double> values = the_agent -> get_values();
            for(double& val: values){
                output_str+=","+std::to_string(val);
            }
        }
        output_str+="\r\n";

        games_played++;
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