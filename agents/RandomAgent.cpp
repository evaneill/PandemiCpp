#include <iostream>

#include "../game_files/GameLogic.h"
#include "../game_files/Actions.h"

#include "RandomAgent.h"
#include "Agents.h"

Agents::RandomAgent::RandomAgent(GameLogic::Game& _active_game): 
    BaseAgent(_active_game){};

Actions::Action* Agents::RandomAgent::generate_action(bool verbose){
    Actions::Action* chosen_action = active_game.get_random_action_uniform(verbose);
    return chosen_action;
}

void Agents::RandomAgent::take_step(bool verbose){
    Actions::Action* chosen_action = generate_action(verbose);// verbose here tells GameLogic whether or not to be verbose
    if(verbose){
        std::cout<< std::endl << "[RandomAgent::take_step()] " << chosen_action -> active_board -> active_player().role.name << ": " << chosen_action -> repr() << std::endl;
    }
    chosen_action -> execute();
}