#include "game_files/GameLogic.h"
#include "game_files/Debug.h"

#include "agents/RandomAgent.h"
/*
... other agent declarations
*/

#include <vector>
#include <iostream>
#include <string>

#include <ctime>
#include <chrono>

void Play(std::vector<int> roles, int difficulty){
    
    // instantiate a game including setup
    DEBUG_MSG("done!" << std::endl << "About to create the game...");
    GameLogic::Game the_game = GameLogic::Game(roles,difficulty,true);
    DEBUG_MSG("done!" << std::endl << "About to create an agent...");

    // // Instantiate an agent pointer with abstract BaseClass
    Agents::BaseAgent* the_agent = new Agents::RandomAgent(the_game);
    DEBUG_MSG("done!" << std::endl << "About to enter the game loop...");
    // While the game isn't over (while maintaining sanity checks throughout)
    int decisions_made = 0;
    
    // While keeping track of sanity checks (at computational expense)
    while(!the_game.is_terminal(true,false)){
        the_agent -> take_step(true);
        decisions_made++;
    }
    DEBUG_MSG(std::endl << "game finished!" << std::endl << "The agent made " + std::to_string(decisions_made) + " decisions.\n");

    DEBUG_MSG("The game ended with a reward of " << the_game.reward() << "\n");
}

int main(){
    // Seed the randomness of the game with current game

    // ===== Thank you stackoverflow =====
    // https://stackoverflow.com/questions/20201141/same-random-numbers-generated-every-time-in-c
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    /* using nano-seconds instead of seconds */
    srand((time_t)ts.tv_nsec);
    // ===== End of stack overflow copypasta ===== 
    
    std::cout << std::endl << "===== STARTING CLOCK =====" << std::endl;
    auto start = std::chrono::high_resolution_clock::now(); 
    // 4 epidemic cards and quarantine specialist, scientist, and researcher
    DEBUG_MSG("About to call Play({0,2,3},4)...");
    Play({0,2,3},4);

    auto stop = std::chrono::high_resolution_clock::now(); 
    std::cout << std::endl << "===== STOPPING CLOCK =====" << std::endl;

    std::chrono::duration<double, std::milli> duration = stop - start;

    // To get the value of duration use the count() 
    // member function on the duration object 

    std::cout << "DURATION (ms): " << duration.count() <<std::endl; 
}

