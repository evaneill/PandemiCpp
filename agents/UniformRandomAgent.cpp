#include <iostream>

#include "../game_files/GameLogic.h"
#include "../game_files/Actions.h"
#include "../game_files/Debug.h"

#include "UniformRandomAgent.h"
#include "Agents.h"

Agents::UniformRandomAgent::UniformRandomAgent(GameLogic::Game& _active_game): 
    BaseAgent(_active_game){};

Actions::Action* Agents::UniformRandomAgent::generate_action(bool verbose){
    Actions::Action* chosen_action = active_game.get_random_action_uniform(verbose);
    return chosen_action;
}

void Agents::UniformRandomAgent::take_step(bool verbose){
    Actions::Action* chosen_action = generate_action(verbose);// verbose here tells GameLogic whether or not to be verbose
    if(verbose){
        DEBUG_MSG(std::endl << "[UniformRandomAgent::take_step()] " << chosen_action -> active_board -> active_player().role.name << ": " << chosen_action -> repr() << std::endl);
    }
    chosen_action -> execute();
}