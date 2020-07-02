#ifndef KSAMPLE_UCTAGENT_H
#define KSAMPLE_UCTAGENT_H

#include "search_tools/Search.h"

#include "../Agents.h"

namespace Agents
{
    class KSample_AStar_SmartCompoundWL_UCTMaxChildAgent: public BaseAgent{
        // simulation-per-step budget
        int n_simulations;

        // Determinizations per stochasticity
        int K;

        // How many visits qualify an average reward as properly "converged" and so worthy of `maxing` over children
        int minVisitConverged;

        // Search tree to use on each step.
        Search::GameTree* search_tree = nullptr;

        // Maximum depth the search tree reached
        std::vector<int> tree_depths ={};

        // TotalReward/N_visits of chosen child
        std::vector<double> chosen_rewards = {};

        // score - (TotalReward/N_Visits) of chosen child
        std::vector<double> chosen_confidences = {};

        // heuristic evaluations of current states
        std::vector<double> state_values = {};

        // Difference between number of times child was visited and (n_simulations/# of children)
        //      (idea is can measure "how different" the chosen action was - very small when there isn't good justification)
        std::vector<double> chosen_visits_minus_avg = {};
    public:
        std::string name = "-Determinization A-star UCT Agent with (# cured diseases/4)+.15* (max fraction of cards held toward curing uncured disease)  + reward for station tangency value rollouts.";
        
        KSample_AStar_SmartCompoundWL_UCTMaxChildAgent(GameLogic::Game& _active_game,int n_simulations,int K,int VisitConvergenceCriteria=100);
        ~KSample_AStar_SmartCompoundWL_UCTMaxChildAgent(){
            if(search_tree){
                delete search_tree;
            }
        };
        Actions::Action* generate_action(bool verbose = false);
        void take_step(bool verbose = false);

        std::vector<std::string>  get_keys();
        std::vector<double>  get_values();

        Search::Node* get_max_child(Search::Node* root);
        double get_max_avgreward(Search::Node* node);

        void reset();
    };
}

#endif