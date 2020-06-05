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
}

Actions::Action* Agents::SingleSampleNaiveUCTAgent::generate_action(bool verbose){
    // Make a new search tree, which will instantiate a root
    Search::GameTree* search_tree = new Search::SingleSampleGameTree(active_game);
    int sims_done = 0;

    while(sims_done<n_simulations){
        // Get most promising leaf for expansion by tree traversal using existing node scores
        Search::Node* best_node = search_tree -> getBestLeaf();
        
        // Rollout the state of the node and delete the COPY (if stochastic) or node state (if not)
        if(best_node -> is_stochastic()){
            // get a *COPY* of the state at the node instead of transitioning in-place
            Board::Board state = best_node -> get_state();

            // Roll out the copy of the state
            // By default rollout will only return W/L
            double reward = active_game.rollout(state);

            // Back up the observed reward, updating node scores with UCB1
            // Deterministic nodes along the way have board_state set to nullptr
            best_node -> backprop(reward,Search::UCB1Score);
        } else {
            // Get the state pointer from the node
            Board::Board* state_ptr = best_node -> get_state_ptr();

            // Roll out the state
            double reward = active_game.rollout(state_ptr);

            // back up the observed reward, updating node scores with UCB1
            // Deterministic nodes along the way have board_state set to nullptr
            best_node -> backprop(reward,Search::UCB1Score);
        }
        sims_done++;
    }

    // After the simulation budget is used up, return the action attached to the most promising child of the root
    return search_tree -> bestAction();
}

void Agents::SingleSampleNaiveUCTAgent::take_step(bool verbose){
    Actions::Action* chosen_action = generate_action(verbose);
    active_game.applyAction(chosen_action);
    if(verbose){
        DEBUG_MSG(std::endl << "[UCTAgent::take_step()] " << active_game.get_board_ptr() -> active_player().role.name << ": " << chosen_action -> repr() << std::endl);
    }
}