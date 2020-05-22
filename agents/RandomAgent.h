#ifndef RANDOM_AGENT_H
#define RANDOM_AGENT_H

#include "../game_files/GameLogic.h"
#include "../game_files/Actions.h"

#include "Agents.h"

// Include new agents in the original Agents namespace; declared only when called
namespace Agents
{
    class RandomAgent: public BaseAgent{
    public:
        RandomAgent(GameLogic::Game& _active_game);
        Actions::Action* generate_action(bool verbose = false);
        void take_step(bool verbose = false);
    };
}

#endif