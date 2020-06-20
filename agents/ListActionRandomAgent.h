#ifndef LISTACTIONRANDOM_AGENT_H
#define LISTACTIONRANDOM_AGENT_H

#include "../game_files/GameLogic.h"
#include "../game_files/Actions.h"

#include "Agents.h"

// Include new agents in the original Agents namespace; declared only when called
namespace Agents
{
    // This agent _should_ be the same as a uniform random agent. The only difference is that they query *all* actions then sample, rather than just straight asking for a random one.
    //      (purely for testing list_actions() in GameLogic)
    class ListActionRandomAgent: public BaseAgent{
    public:
        ListActionRandomAgent(GameLogic::Game& _active_game);
        Actions::Action* generate_action(bool verbose = false);
        void take_step(bool verbose = false);

        std::vector<std::string>  get_keys();
        std::vector<double>  get_values();

        void reset();
    };
}

#endif