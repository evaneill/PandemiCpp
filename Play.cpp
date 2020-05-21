#include "game_files/GameLogic.h"

#include "agents/RandomAgent.h"
/*
... other agent declarations
*/

#include <vector>
#include <iostream>
#include <string>

#include <ctime>

void Play(std::vector<int> roles, int difficulty){
    
    // instantiate a game including setup
    std::cout << "done!" << std::endl << "About to create the game...";
    GameLogic::Game the_game = GameLogic::Game(roles,difficulty,true);
    std::cout << "done!" << std::endl << "About to create an agent...";

    // // Instantiate an agent pointer with abstract BaseClass
    Agents::BaseAgent* the_agent = new Agents::RandomAgent(the_game);
    std::cout << "done!" << std::endl << "About to enter the game loop...";
    // While the game isn't over (while maintaining sanity checks throughout)
    int decisions_made = 0;
    
    // While keeping track of sanity checks (at computational expense)
    while(!the_game.is_terminal(false)){
        the_agent -> take_step(true);
        decisions_made++;
    }
    std::cout << std::endl << "game finished!" << std::endl << "The agent made " + std::to_string(decisions_made) + " decisions.\n";
    std::cout << "The game ended with a reward of " << the_game.reward() << "\n";
}

int main(){
    // Seed the randomness of the game with current game
    std::srand(std::time(NULL));
    rand();
    
    // 4 epidemic cards and quarantine specialist, scientist, and researcher
    std::cout << "About to call Play({0,2,3},4)...";
    Play({0,2,3},4);
}

