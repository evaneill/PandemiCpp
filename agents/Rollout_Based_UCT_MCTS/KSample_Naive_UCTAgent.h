#ifndef KSAMPLE_UCTAGENT_H
#define KSAMPLE_UCTAGENT_H

#include "search_tools/Search.h"

#include "../Agents.h"

namespace Agents
{
    class KSample_Naive_UCTAgent: public BaseAgent{
        // simulation-per-step budget
        int n_simulations;

        // Determinizations per stochasticity
        int K;

        // Search tree to use on each step.
        Search::GameTree* search_tree = nullptr;

        // Maximum depth the search tree reached
        std::vector<int> tree_depths ={};

        // TotalReward/N_visits of chosen child
        std::vector<double> chosen_rewards = {};

        // score - (TotalReward/N_Visits) of chosen child
        std::vector<double> chosen_confidences = {};

        // vector of choice node average rewards backed up through the tree
        std::vector<double> state_values = {};

        // Difference between number of times child was visited and (n_simulations/# of children)
        //      (idea is can measure "how different" the chosen action was - very small when there isn't good justification)
        std::vector<double> chosen_visits_minus_avg = {};
    public:
        std::string name = "-Determinization UCT Agent with 0/1 value rollouts.";
        
        KSample_Naive_UCTAgent(GameLogic::Game& _active_game,int n_simulations,int K);
        ~KSample_Naive_UCTAgent(){
            if(search_tree){
                delete search_tree;
            }
        };
        Actions::Action* generate_action(bool verbose = false);
        void take_step(bool verbose = false);

        std::vector<std::string>  get_keys();
        std::vector<double>  get_values();

        void reset();
    };
}

#endif