#ifndef KSAMPLE_UCTAGENT_H
#define KSAMPLE_UCTAGENT_H

#include "search_tools/Search.h"

#include "Agents.h"

namespace Agents
{
    class KSampleNaiveUCTAgent: public BaseAgent{
        // simulation-per-step budget
        int n_simulations;

        // Determinizations per stochasticity
        int K;

        // Search tree to use on each step.
        Search::GameTree* search_tree = nullptr;
    public:
        std::string name = "Single-Stochastic-Sample UCT Agent with 0/1 value rollouts.";
        
        KSampleNaiveUCTAgent(GameLogic::Game& _active_game,int n_simulations,int K);
        ~KSampleNaiveUCTAgent(){
            if(search_tree){
                delete search_tree;
            }
        };
        Actions::Action* generate_action(bool verbose = false);
        void take_step(bool verbose = false);
    };
}

#endif