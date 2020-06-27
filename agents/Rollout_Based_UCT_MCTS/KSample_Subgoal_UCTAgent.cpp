#include <cmath>
#include <algorithm>

#include "../game_files/Board.h"
#include "../game_files/Debug.h"
#include "../game_files/GameLogic.h"

#include "../Agents.h"
#include "../Heuristics.h"
#include "KSample_Subgoal_UCTAgent.h"

#include "search_tools/Search.h"

Agents::KSample_Subgoal_UCTAgent::KSample_Subgoal_UCTAgent(GameLogic::Game& _active_game, int _n_simulations, int _K):
    BaseAgent(_active_game)
    {   
        n_simulations = _n_simulations;

        K = _K;

        name += "(" + std::to_string(n_simulations) + " simulations per step)";
        name = std::to_string(K) + " " + name;

        measurable=true;
}

Actions::Action* Agents::KSample_Subgoal_UCTAgent::generate_action(bool verbose){
    // Make a new search tree, which will instantiate a root
    // This tree takes 1 sample of a stochastic sequence at each stochastic node
    // Stochasticity is "saved" and revisited again upon every subsequent traversal
    
    // This represents a potential memory leak since without being used in take_step, tree at any existing ptr won't be deleted
    search_tree = new Search::KDeterminizedGameTree(active_game,K);
    int sims_done = 0;

    int search_depth=0;

    while(sims_done<n_simulations){   
        // Make a copy of the current board state to hand to the tree for consideration
        Board::Board board_copy = active_game.board_copy();

        // Give the copy state to the tree in order for it to "roll it down" the actions on the tree to end up at a new deterministic node
        // Alter the board_copy in place to end up at a "leaf" board position
        // Return the best node resulting from tree policy
        Search::Node* best_node = search_tree -> getBestLeaf(board_copy,Search::UCB1Score);

        // Update the tree depth
        if(best_node -> depth > search_depth){
            search_depth=best_node -> depth;
        }

        // Roll out the copy of the state
        // Use the "CureGoalHeuristic": What fraction of 4 diseases are cured at rollout end
        double reward = active_game.rollout(board_copy,Heuristics::CureGoalHeuristic);

        // Back up the observed reward
        // Deterministic nodes along the way have board_state set to nullptr
        best_node -> backprop(reward);
        
        // One more simulation succesfully ran
        sims_done++;
    }

    Search::Node* chosen_child = search_tree -> bestRootChild(Search::UCB1Score);

    tree_depths.push_back(search_depth); // Maximum search depth at this step
    chosen_rewards.push_back((double) chosen_child -> TotalReward / (double) chosen_child -> N_visits); // "Expected Reward"
    chosen_confidences.push_back(Search::UCB1Score(chosen_child)-((double) chosen_child -> TotalReward / (double) chosen_child -> N_visits)); // "Exploration term" = upper confidence interval size; avoiding calculating it for ourselves.
    chosen_visits_minus_avg.push_back((
        (
            (double) chosen_child -> N_visits) - ((double) n_simulations/(double)chosen_child -> parent -> n_children())
        )/ (double) n_simulations); // Fraction of all simulations spent on this choice compared to a child explored an "average" amount. Neg -> This was chosen after other heavily explored options ruled out. Pos -> This was chosen after thorough exploration. ~0 -> Most likely no good reason to choose this vs. other children.

    // After the simulation budget is used up, return the action attached to the most promising child of the root
    return chosen_child -> get_action();
}

void Agents::KSample_Subgoal_UCTAgent::take_step(bool verbose){
    Actions::Action* chosen_action = generate_action(verbose);
    active_game.applyAction(chosen_action);
    if(verbose){
        DEBUG_MSG(std::endl << "[UCTAgent::take_step()] " << active_game.get_board_ptr() -> active_player().role.name << ": " << chosen_action -> repr() << std::endl);
    }
    // Get rid of the search tree if it exists, which will include deletion of chosen_action
    if(search_tree){
        delete search_tree;
        search_tree=nullptr;
    }
}

void Agents::KSample_Subgoal_UCTAgent::reset(){
    tree_depths.clear();
    chosen_rewards.clear();
    chosen_confidences.clear();
    chosen_visits_minus_avg.clear();
}

std::vector<std::string> Agents::KSample_Subgoal_UCTAgent::get_keys(){
    return {
        "AvgTreeDepth",
        "StdTreeDepth",
        "MaxTreeDepth",
        "MinTreeDepth",
        "AvgSelectedReward",
        "StdSelectedReward",
        "MaxSelectedReward",
        "MinSelectedReward",
        "AvgSelectedConfidence",
        "StdSelectedConfidence",
        "MaxSelectedConfidence",
        "MinSelectedConfidence",
        "AvgChosenMinusAvgVisits",
        "MaxChosenMinusAvgVisits"
    };
}

std::vector<double> Agents::KSample_Subgoal_UCTAgent::get_values(){
    // Avg tree depth
    double depth_mean=0;
    for(int& d : tree_depths){
        depth_mean+=(double) d/ (double) tree_depths.size();
    }

    // std tree depth
    double depth_std=0;
    for(int& d : tree_depths){
        depth_std+=std::pow((double) d -depth_mean,2);
    }
    depth_std = std::pow(depth_std,.5);

    // Avg reward
    double reward_mean=0;
    for(double& r : chosen_rewards){
        reward_mean+=r/ (double) chosen_rewards.size();
    }

    // std reward
    double reward_std=0;
    for(double& r : chosen_rewards){
        reward_std+=std::pow(r - reward_mean,2);
    }
    reward_std = std::pow(reward_std,.5);

    // Avg confidence
    double confidence_mean=0;
    for(double& c : chosen_confidences){
        confidence_mean+= c/ (double) chosen_confidences.size();
    }

    // std confidence
    double confidence_std=0;
    for(double& c : chosen_confidences){
        confidence_std+=std::pow(c - confidence_mean,2);
    }
    confidence_std = std::pow(confidence_std,.5);

    // avg visits - avg visits
    double visits_minus_avg_mean =0 ;
    for(double& v: chosen_visits_minus_avg){
        visits_minus_avg_mean+= v / (double) chosen_visits_minus_avg.size();
    }

    return {
        depth_mean, // Average Tree-Search max depth
        depth_std, // Std Tree-search max depth
        (double) *std::max_element(tree_depths.begin(),tree_depths.end()), // Max Tree-search max depth
        (double) *std::min_element(tree_depths.begin(),tree_depths.end()), // Min Tree-search max depth
        reward_mean, // Average expected reward of chosen children
        reward_std, // std of expected reward of chosen children
        *std::max_element(chosen_rewards.begin(),chosen_rewards.end()), // Max chosen reward
        *std::min_element(chosen_rewards.begin(),chosen_rewards.end()), // Min chosen reward
        confidence_mean, // average confidence interval size on chosen child
        confidence_std, // std confidence interval size on chosen child
        *std::max_element(chosen_confidences.begin(),chosen_confidences.end()), // Max confidence bound size
        *std::min_element(chosen_confidences.begin(),chosen_confidences.end()), // Min confidence bound size
        visits_minus_avg_mean,
        *std::max_element(chosen_visits_minus_avg.begin(),chosen_visits_minus_avg.end())
    };  
}