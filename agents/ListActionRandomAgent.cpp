#include <iostream>

#include "../game_files/GameLogic.h"
#include "../game_files/Actions.h"
#include "../game_files/Debug.h"

#include "ListActionRandomAgent.h"
#include "Agents.h"

Agents::ListActionRandomAgent::ListActionRandomAgent(GameLogic::Game& _active_game): 
    BaseAgent(_active_game){};

Actions::Action* Agents::ListActionRandomAgent::generate_action(bool verbose){
    std::vector<Actions::Action*> action_list = active_game.list_actions(verbose);
    Actions::Action* chosen_action = action_list[rand() % action_list.size()];
    return chosen_action;
}

void Agents::ListActionRandomAgent::take_step(bool verbose){
    Actions::Action* chosen_action = generate_action(verbose);// verbose here tells GameLogic whether or not to be verbose
    if(verbose){
        DEBUG_MSG(std::endl << "[ListActionRandomAgent::take_step()] " << active_game.get_board_ptr() -> active_player().role.name << ": " << chosen_action -> repr() << std::endl);
    }
    active_game.applyAction(chosen_action);
}