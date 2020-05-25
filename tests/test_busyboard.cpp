#include "../game_files/GameLogic.h"
#include "../game_files/Debug.h"

#include "../scenarios/Scenarios.h"

#include "../agents/ByGroupRandomAgent.h"
/*
... other agent declarations
*/

#include <vector>
#include <iostream>
#include <string>

#include <ctime>
#include <chrono>

void Play(){
    
    // instantiate a game including setup. This should be referenced by all subsequent entities
    DEBUG_MSG("done!" << std::endl << "About to create the game...");
    Board::Board busyboard_test = Scenarios::BusyBoardTest(true);
    
    // Give the game to the GameLogic
    GameLogic::Game the_game = GameLogic::Game(busyboard_test);
    DEBUG_MSG("done!" << std::endl << "About to create an agent...");

    // // Instantiate an agent pointer with abstract BaseClass
    Agents::BaseAgent* the_agent = new Agents::ByGroupRandomAgent(the_game);
    DEBUG_MSG("done!" << std::endl << "About to enter the game loop...");
    int decisions_made = 0;
    
    // IGNORE sanity checks since I'm giving players duplicate cards
    while(!the_game.is_terminal(false)){
        // First resolve any necessary non-player transisitions (card draws, etc)
        the_game.nonplayer_actions(true);
        
        // If these transitions haven't made the game terminal, then have the agent choose a transition
        if(!the_game.is_terminal(false)){
            the_agent -> take_step(true);
            decisions_made++;
        }
    }
    DEBUG_MSG(std::endl << "[Play()] game finished!" << std::endl << "The agent made " + std::to_string(decisions_made) + " decisions.\n");

    DEBUG_MSG("[Play()] The game ended with a reward of " << the_game.reward() << "\n");
    DEBUG_MSG("[Play()] Terminal reasons: " << std::endl);
    for(std::string reason: the_game.terminal_reasons()){
        DEBUG_MSG("[Play()] ..." << reason << std::endl);
    }
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
    DEBUG_MSG("About to call Play(s)...");
    Play();

    auto stop = std::chrono::high_resolution_clock::now(); 
    std::cout << std::endl << "===== STOPPING CLOCK =====" << std::endl;

    std::chrono::duration<double, std::milli> duration = stop - start;

    // To get the value of duration use the count() 
    // member function on the duration object 

    std::cout << "DURATION (ms): " << duration.count() <<std::endl; 
}

