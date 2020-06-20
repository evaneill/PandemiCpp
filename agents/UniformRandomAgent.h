#ifndef UNIFORMRANDOM_AGENT_H
#define UNIFORMRANDOM_AGENT_H

#include "../game_files/GameLogic.h"
#include "../game_files/Actions.h"

#include "Agents.h"

// Include new agents in the original Agents namespace; declared only when called
namespace Agents
{
    class UniformRandomAgent: public BaseAgent{
    public:
        std::string name = "UniformRandomAgent";

        UniformRandomAgent(GameLogic::Game& _active_game);
        ~UniformRandomAgent(){};
        Actions::Action* generate_action(bool verbose = false);
        void take_step(bool verbose = false);

        std::vector<std::string>  get_keys();
        std::vector<double>  get_values();

        void reset();
    };
}

#endif