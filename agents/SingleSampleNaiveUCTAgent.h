#ifndef UCTAGENT_H
#define UCTAGENT_H

#include "search_tools/Search.h"

#include "Agents.h"

namespace Agents
{
    class SingleSampleNaiveUCTAgent: public BaseAgent{
        // simulation-per-step budget
        int n_simulations;
    public:
        std::string name = "Single-Sample UCT Agent with 0/1 value rollouts";
        
        SingleSampleNaiveUCTAgent(GameLogic::Game& _active_game,int n_simulations);
        ~SingleSampleNaiveUCTAgent(){};
        Actions::Action* generate_action(bool verbose = false);
        void take_step(bool verbose = false);
    };
}

#endif