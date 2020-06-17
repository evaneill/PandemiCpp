#ifndef KSAMPLE_UCTAGENT_H
#define KSAMPLE_UCTAGENT_H

#include "search_tools/Search.h"

#include "Agents.h"

namespace Agents
{
    class KSampleSubGoalHeuristicUCTAgent: public BaseAgent{
        // simulation-per-step budget
        int n_simulations;

        // Determinizations per stochasticity
        int K;

        // Search tree to use on each step.
        Search::GameTree* search_tree = nullptr;
    public:
        std::string name = "-Determinization UCT Agent with (# cured diseases/4) value rollouts.";
        
        KSampleSubGoalHeuristicUCTAgent(GameLogic::Game& _active_game,int n_simulations,int K);
        ~KSampleSubGoalHeuristicUCTAgent(){
            if(search_tree){
                delete search_tree;
            }
        };
        Actions::Action* generate_action(bool verbose = false);
        void take_step(bool verbose = false);
    };
}

#endif