#include <cmath>
#include <algorithm>

#include "../game_files/Board.h"
#include "../game_files/Debug.h"
#include "../game_files/GameLogic.h"

#include "../Agents.h"
#include "../Heuristics.h"
#include "KSample_CompoundWL_UCTMaxChildAgent.h"

#include "../search_tools/Search.h"

Agents::KSample_CompoundWL_UCTMaxChildAgent::KSample_CompoundWL_UCTMaxChildAgent(GameLogic::Game& _active_game, int _n_simulations, int _K,int _VisitConvergenceCriteria):
    BaseAgent(_active_game)
    {   
        n_simulations = _n_simulations;

        K = _K;
        minVisitConverged = _VisitConvergenceCriteria;

        name += "(" + std::to_string(n_simulations) + " simulations per step)";
        name = std::to_string(K) + " " + name;

        measurable=true;
}

Actions::Action* Agents::KSample_CompoundWL_UCTMaxChildAgent::generate_action(bool verbose){
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

        // Straight up evaluate the heuristic of the state
        // Use the "CureGoalConditions": What fraction of 4 diseases are cured at rollout end, PLUS maximum fraction of satisfied preconditions to cure actions among players
        double reward = Heuristics::CompoundHeuristic(board_copy,Heuristics::CureGoalConditionswStation,Heuristics::LossProximity);

        // Back up the observed reward
        // (terminal nodes never get value changed on backprop)
        best_node -> backprop(reward);
        
        // One more simulation succesfully ran
        sims_done++;
    }

    Search::Node* chosen_child = get_max_child(search_tree -> root);

    tree_depths.push_back(search_depth); // Maximum search depth at this step
    chosen_rewards.push_back((double) chosen_child -> TotalReward / (double) chosen_child -> N_visits); // "Expected Reward"
    chosen_confidences.push_back(Search::UCB1Score(chosen_child)-((double) chosen_child -> TotalReward / (double) chosen_child -> N_visits)); // "Exploration term" = upper confidence interval size
    chosen_visits_minus_avg.push_back((
        (
            (double) chosen_child -> N_visits) - ((double) n_simulations/(double)chosen_child -> parent -> n_children()) 
        )/ (double) n_simulations); // Fraction of all simulations spent on this choice compared to a child explored an "average" amount. Neg -> This was chosen after other heavily explored options ruled out. Pos -> This was chosen after thorough exploration. ~0 -> Most likely no good reason to choose this vs. other children.

    Board::Board board_copy = active_game.board_copy();
    state_values.push_back(Heuristics::CompoundHeuristic(board_copy,Heuristics::CureGoalConditionswStation,Heuristics::LossProximity));
    // After the simulation budget is used up, return the action attached to the most promising child of the root
    return chosen_child -> get_action();
}

Search::Node* Agents::KSample_CompoundWL_UCTMaxChildAgent::get_max_child(Search::Node* root){
    // This is to be called on root, which is a deterministic node
    // This represents GREEDY SELECTION of maximum reward
    Search::Node* best_child = nullptr;
    double best_child_reward = -1;

    // Go through children and find the one with the highest max_reward, using the agent-level convergence criterion.
    for(int child_num=0;child_num<root -> n_children();child_num++){
        Search::Node* child = root -> getChild(child_num);
        double child_score = get_max_avgreward(child);

        // Set totalReward based on max "acheivable" reward expectation
        child -> TotalReward = child_score * (child -> N_visits);

        // Set the score w/ UCB on newly defined total reward (if terminal, keep game score of the board)
        child -> update(Search::UCB1Score);

        // If (max avg reward) > best existing reward
        if(child_score >best_child_reward){
            best_child = child;
            best_child_reward = child_score;
        }
    }
    return best_child;
}

double Agents::KSample_CompoundWL_UCTMaxChildAgent::get_max_avgreward(Search::Node* node){
    if(node -> terminal){
        // If the node is terminal it's average reward is it's only reward is a for-sure reward is the score that was set on instantiation
        return node -> score;
    }
    if(!node -> converged(minVisitConverged)){
        // If the node isn't converged, just return the total reward seen on all traversals of this node
        return (node -> TotalReward)/((double) node -> N_visits);
    } else {
        // Otherwise this node is converged, and we handle stochastic and deterministic nodes differently
        //
        // Deterministic -> max avg_reward of children
        // Stochastic -> Equally weighted max avg_reward of deterministic successors, for those that exist

        if(!node -> stochastic){
            // Deterministic: get max reward of each child, and return the maximum value
            double best_score = -1;
            for(int child_num=0;child_num<node -> n_children();child_num++){
                Search::Node* child = node -> getChild(child_num);
                double child_max_score = get_max_avgreward(child);
                if(child_max_score>best_score){
                    best_score = child_max_score;
                }
            }
            return best_score;
        } else {
            // Stochastic: for each deterministic child that exists, add max reward weighted by # of determinizations traversed
            double avg_maxreward = 0;
            int existing_children = 0;

            for(int child_num=0;child_num<node -> n_children();child_num++){
                Search::Node* child = node -> getChild(child_num);
                // get `child_num`th child
                if(child){
                    // If it isn't a nullptr then this determinization has been visited before.
                    existing_children++;
                    
                    // Traverse nodes down to next deterministic/terminal node and add it's max value
                    while(child -> stochastic && !child -> terminal){
                        // Always 0th child because we're using a KDeterminizedTree, that uses this kind of structure
                        child = child -> getChild(0);
                    }
                    // Add max child reward to the total of max reward seen under this stochasticity
                    avg_maxreward+=get_max_avgreward(child);
                }
            }
            return avg_maxreward/((double) existing_children);
        }
    }
}

void Agents::KSample_CompoundWL_UCTMaxChildAgent::take_step(bool verbose){
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

void Agents::KSample_CompoundWL_UCTMaxChildAgent::reset(){
    tree_depths.clear();
    chosen_rewards.clear();
    chosen_confidences.clear();
    chosen_visits_minus_avg.clear();
    state_values.clear();
}

std::vector<std::string> Agents::KSample_CompoundWL_UCTMaxChildAgent::get_keys(){
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
        "MaxChosenMinusAvgVisits",
        "Turn0SelectedReward",
        "Turn5SelectedReward",
        "Turn10SelectedReward",
        "Turn15SelectedReward",
        "Turn20SelectedReward",
        "Turn25SelectedReward",
        "Turn30SelectedReward",
        "Turn35SelectedReward",
        "Turn40SelectedReward",
        "Turn45SelectedReward",
        "Turn50SelectedReward",
        "Turn55SelectedReward",
        "Turn60SelectedReward",
        "Turn65SelectedReward",
        "Turn70SelectedReward",
        "Turn75SelectedReward",
        "Turn80SelectedReward",
        "Turn85SelectedReward",
        "Turn90SelectedReward",
        "Turn0StateEval",
        "Turn5StateEval",
        "Turn10StateEval",
        "Turn15StateEval",
        "Turn20StateEval",
        "Turn25StateEval",
        "Turn30StateEval",
        "Turn35StateEval",
        "Turn40StateEval",
        "Turn45StateEval",
        "Turn50StateEval",
        "Turn55StateEval",
        "Turn60StateEval",
        "Turn65StateEval",
        "Turn70StateEval",
        "Turn75StateEval",
        "Turn80StateEval",
        "Turn85StateEval",
        "Turn90StateEval"
    };
}

std::vector<double> Agents::KSample_CompoundWL_UCTMaxChildAgent::get_values(){
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
        *std::max_element(chosen_visits_minus_avg.begin(),chosen_visits_minus_avg.end()),
        chosen_rewards.size()>=(0+1) ? chosen_rewards[0] : -1.,
        chosen_rewards.size()>=(5+1) ? chosen_rewards[5] : -1.,
        chosen_rewards.size()>=(10+1) ? chosen_rewards[10] : -1.,
        chosen_rewards.size()>=(15+1) ? chosen_rewards[15] : -1.,
        chosen_rewards.size()>=(20+1) ? chosen_rewards[20] : -1.,
        chosen_rewards.size()>=(25+1) ? chosen_rewards[25] : -1.,
        chosen_rewards.size()>=(30+1) ? chosen_rewards[30] : -1.,
        chosen_rewards.size()>=(35+1) ? chosen_rewards[35] : -1.,
        chosen_rewards.size()>=(40+1) ? chosen_rewards[40] : -1.,
        chosen_rewards.size()>=(45+1) ? chosen_rewards[45] : -1.,
        chosen_rewards.size()>=(50+1) ? chosen_rewards[50] : -1.,
        chosen_rewards.size()>=(55+1) ? chosen_rewards[55] : -1.,
        chosen_rewards.size()>=(60+1) ? chosen_rewards[60] : -1.,
        chosen_rewards.size()>=(65+1) ? chosen_rewards[65] : -1.,
        chosen_rewards.size()>=(70+1) ? chosen_rewards[70] : -1.,
        chosen_rewards.size()>=(75+1) ? chosen_rewards[75] : -1.,
        chosen_rewards.size()>=(80+1) ? chosen_rewards[80] : -1.,
        chosen_rewards.size()>=(85+1) ? chosen_rewards[85] : -1.,
        chosen_rewards.size()>=(90+1) ? chosen_rewards[90] : -1.,
        state_values.size()>=(0+1) ? state_values[0] : -1,
        state_values.size()>=(5+1) ? state_values[5] : -1,
        state_values.size()>=(10+1) ? state_values[10] : -1,
        state_values.size()>=(15+1) ? state_values[15] : -1,
        state_values.size()>=(20+1) ? state_values[20] : -1,
        state_values.size()>=(25+1) ? state_values[25] : -1,
        state_values.size()>=(30+1) ? state_values[30] : -1,
        state_values.size()>=(35+1) ? state_values[35] : -1,
        state_values.size()>=(40+1) ? state_values[40] : -1,
        state_values.size()>=(45+1) ? state_values[45] : -1,
        state_values.size()>=(50+1) ? state_values[50] : -1,
        state_values.size()>=(55+1) ? state_values[55] : -1,
        state_values.size()>=(60+1) ? state_values[60] : -1,
        state_values.size()>=(65+1) ? state_values[65] : -1,
        state_values.size()>=(70+1) ? state_values[70] : -1,
        state_values.size()>=(75+1) ? state_values[75] : -1,
        state_values.size()>=(80+1) ? state_values[80] : -1,
        state_values.size()>=(85+1) ? state_values[85] : -1,
        state_values.size()>=(90+1) ? state_values[90] : -1
    };  
}