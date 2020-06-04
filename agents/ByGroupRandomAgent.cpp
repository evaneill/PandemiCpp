#include <iostream>

#include "../game_files/GameLogic.h"
#include "../game_files/Actions.h"
#include "../game_files/Debug.h"

#include "ByGroupRandomAgent.h"
#include "Agents.h"

Agents::ByGroupRandomAgent::ByGroupRandomAgent(GameLogic::Game& _active_game): 
    BaseAgent(_active_game){};

Actions::Action* Agents::ByGroupRandomAgent::generate_action(bool verbose){
    Actions::Action* chosen_action = active_game.get_random_action_bygroup(verbose);
    return chosen_action;
}

void Agents::ByGroupRandomAgent::take_step(bool verbose){
    Actions::Action* chosen_action = generate_action(verbose);// verbose here tells GameLogic whether or not to be verbose
    active_game.applyAction(chosen_action);
    if(verbose){
        DEBUG_MSG(std::endl << "[ByGroupRandomAgent::take_step()] " << active_game.get_board_ptr() -> active_player().role.name << ": " << chosen_action -> repr() << std::endl);
    }
}