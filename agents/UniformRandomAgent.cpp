#include <iostream>

#include "../game_files/GameLogic.h"
#include "../game_files/Actions.h"
#include "../game_files/Debug.h"

#include "UniformRandomAgent.h"
#include "Agents.h"

Agents::UniformRandomAgent::UniformRandomAgent(GameLogic::Game& _active_game): 
    BaseAgent(_active_game){
        measurable=false;
};

Actions::Action* Agents::UniformRandomAgent::generate_action(bool verbose){
    Actions::Action* chosen_action = active_game.get_random_action_uniform(verbose);
    return chosen_action;
}

void Agents::UniformRandomAgent::take_step(bool verbose){
    Actions::Action* chosen_action = generate_action(verbose);// verbose here tells GameLogic whether or not to be verbose
    active_game.applyAction(chosen_action);
    if(verbose){
        DEBUG_MSG(std::endl << "[UniformRandomAgent::take_step()] " << active_game.get_board_ptr() -> active_player().role.name << ": " << chosen_action -> repr() << std::endl);
    }
    delete chosen_action;
}

// These logically shouldn't be called, but these returns should be safe anyway
std::vector<std::string> Agents::UniformRandomAgent::get_keys(){return {};};
std::vector<double> Agents::UniformRandomAgent::get_values(){return {};};

void Agents::UniformRandomAgent::reset(){};