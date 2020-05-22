#include "Agents.h"
#include "../game_files/GameLogic.h"

Agents::BaseAgent::BaseAgent(GameLogic::Game& _active_game):
    active_game(_active_game){}
