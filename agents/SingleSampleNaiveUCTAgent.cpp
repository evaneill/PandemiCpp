#include "../game_files/Board.h"
#include "../game_files/Debug.h"
#include "../game_files/GameLogic.h"

#include "Agents.h"
#include "Heuristics.h"
#include "SingleSampleNaiveUCTAgent.h"

#include "search_tools/Search.h"

Agents::SingleSampleNaiveUCTAgent::SingleSampleNaiveUCTAgent(GameLogic::Game& _active_game, int _n_simulations):
    BaseAgent(_active_game)
    {
    n_simulations = _n_simulations;
    name += "(" + std::to_string(n_simulations) + " simulations per step)";
}

Actions::Action* Agents::SingleSampleNaiveUCTAgent::generate_action(bool verbose){
    // Make a new search tree, which will instantiate a root
    // This tree takes 1 sample of a stochastic sequence at each stochastic node
    // Stochasticity is "saved" and revisited again upon every subsequent traversal
    Search::GameTree* search_tree = new Search::KSampleGameTree(active_game,1);
    int sims_done = 0;

    while(sims_done<n_simulations){

        // Make a copy of the current board state to hand to the tree for consideration
        Board::Board board_copy = active_game.board_copy();

        // Give the copy state to the tree in order for it to "roll it down" the actions on the tree to end up at a new deterministic node
        // Alter the board_copy in place to end up at a "leaf" board position
        // Return the best node resulting from tree policy
        Search::Node* best_node = search_tree -> getBestLeaf(board_copy);

        // Roll out the copy of the state
        // By default rollout will only return W/L
        double reward = active_game.rollout(board_copy);

        // Back up the observed reward, updating node scores with UCB1
        // Deterministic nodes along the way have board_state set to nullptr
        best_node -> backprop(reward,Search::UCB1Score);

        // One more simulation succesfully ran
        sims_done++;
    }

    Actions::Action* chosen_action = search_tree -> bestAction();
    
    // Get rid of the search tree
    delete search_tree;

    // After the simulation budget is used up, return the action attached to the most promising child of the root
    return chosen_action;
}

void Agents::SingleSampleNaiveUCTAgent::take_step(bool verbose){
    Actions::Action* chosen_action = generate_action(verbose);
    active_game.applyAction(chosen_action);
    if(verbose){
        DEBUG_MSG(std::endl << "[UCTAgent::take_step()] " << active_game.get_board_ptr() -> active_player().role.name << ": " << chosen_action -> repr() << std::endl);
    }
    delete chosen_action;
}