#include <cmath>

#include "Search.h"

#include "../game_files/Actions.h"
#include "../game_files/GameLogic.h"
#include "../game_files/Board.h"

double Search::UCB1Score(Search::Node* node){
    // Take a node and number of simulations performed so far by this agent in considering the curreng game state
    //  (n_sim to be used when parent is nullptr)
    // return UCB1 score = (avg reward seen over all visits) + sqrt(2*ln(# parent visits)/(# child visits))
    double value=0;
    value+= ((double) node -> TotalReward / (double) node -> N_visits); // average reward

    int n_parent_visits;
    if(!node -> get_parent()){
        // If no parent (its the root), they're the same value and this calculation won't matter anyway
        n_parent_visits = node -> N_visits;
    } else {
        n_parent_visits = node -> get_parent() -> N_visits;
    }

    // sqrt(2*ln(# parent visits)/(# child visits))
    value+=std::pow(2*std::log((double) n_parent_visits)/(double) node -> N_visits,.5);

    return value;
}

Search::Node::Node(Node* _parent,Actions::Action* _action){
    parent = _parent;
    action = _action;
}

Search::DeterministicNode::DeterministicNode(Search::Node* _parent,Actions::Action* _action, Board::Board* _board_state,GameLogic::Game& game_logic):
    Node(_parent,_action){
    // An instantiated node is always labeled with terminal = true if the state to which it's associated is terminal
    if(game_logic.is_terminal(*_board_state)){
        terminal = true;
        action_queue = {};
    } else {
        terminal=false;
        action_queue = game_logic.list_actions(*_board_state);
    }

    stochastic=false;

    // Always initialize empty children on a deterministic new node
    children = {};
}

void Search::DeterministicNode::set_score(double score_fn(Search::Node*)){
    if(!terminal){
        score = score_fn(this);
    }
    // if terminal, score should have been instantiated on creation and never change.
}

void Search::DeterministicNode::update(double reward,double score_fn(Search::Node*)){
    TotalReward+=reward;
    N_visits+=1;

    set_score(score_fn);
}

void Search::DeterministicNode::backprop(double reward,double score_fn(Search::Node*)){
    update(reward,score_fn);
    if(parent){
        // If parent isn't nullptr, tell the parent to backprop
        parent -> backprop(reward,score_fn);
    }
}

Actions::Action* Search::DeterministicNode::get_action(){
    return action;
}

int Search::DeterministicNode::n_children(){
    return children.size();
}

bool Search::DeterministicNode::is_terminal(){
    return terminal;
}

bool Search::DeterministicNode::is_stochastic(){
    return stochastic;
}

Search::Node* Search::DeterministicNode::get_parent(){
    return parent;
}

Search::Node* Search::DeterministicNode::best_child(Board::Board& state,GameLogic::Game& game_logic){
    if(!terminal){
        if(action_queue.empty()){
            // If there are no untried actions, always return the best child as per score that's been being updated
            
            // Necessary to force the queue to update tho
            Search::Node* was_best_before = children.top();
            children.pop();
            children.push(was_best_before);
            
            // Use the top of the queue to advance the state in place
            children.top() -> get_action() -> execute(state);

            // Return the best child after having updating the queue
            return children.top();
        } else {
            // Otherwise pull an action out of the stack
            Actions::Action* new_action = action_queue.back();
            
            // alter the state in place
            new_action -> execute(state); 

            // Delete the action from the stack
            action_queue.pop_back();

            // And define a new node with this action
            Search::Node* new_node;
            if(game_logic.is_stochastic(state)){
                // stochastic nodes for states requiring game logic in order to advance
                new_node = new Search::StochasticNode(this,new_action,&state,game_logic);
            } else {
                // deterministic nodes that need player choice
                new_node = new Search::DeterministicNode(this,new_action,&state,game_logic); 
            }
            // And insert it into children for later
            children.push(new_node);

            // And return it as the "best child"
            return new_node;
        }
    }
    // should break if a terminal node is being asked for children
    return nullptr;
}

void Search::DeterministicNode::addChild(Board::Board& game_board, Actions::Action* action,GameLogic::Game& game_logic){};

Search::Node* Search::DeterministicNode::getChild(int child){return nullptr;};

int Search::StochasticNode::n_children(){
    return children.size();
};

Search::StochasticNode::StochasticNode(Search::Node* _parent,Actions::Action* _action, Board::Board* _board_state,GameLogic::Game& game_logic):
    Node(_parent,_action){
        stochastic = true;
        
        if(game_logic.is_terminal(*_board_state)){
            terminal = true;
        }

        // I'm choosing to implement stochastic nodes with no children on the constructor. It's up to the tree that's using such nodes to decide what to do with them.
        children = {};
    }

Search::Node* Search::StochasticNode::best_child(Board::Board& state,GameLogic::Game& game_logic){
    // Goal is to get to the next deterministic node
    // Requires the tree to have made a choice about the child structure of stochastic nodes returned during tree policy search
    if(!terminal){
        if(!children.empty()){

            // Pick a random successor
            Search::Node* best_child = children[rand() % children.size()];

            // Apply its action to the given state
            best_child -> get_action() -> execute(state);

            // Follow it down the rabbit hole until you have a deterministic or terminal node
            if(best_child -> stochastic && !best_child -> terminal){
                // recursive call to best_child is *maybe* questionable, but I think in general forcing randomness (randomness over children) should be inherent to stochastic nodes
                // Only the structure of the tree seems like should be able to control stochastic behavior
                best_child = best_child -> best_child(state,game_logic);
            }
            // return a terminal or deterministic node
            return best_child;
        }
    }
    // should break if a node with no children or terminal node is asked for a child
    return nullptr;
}

Actions::Action* Search::StochasticNode::get_action(){
    return action;
}

bool Search::StochasticNode::is_terminal(){
    return terminal;
}

bool Search::StochasticNode::is_stochastic(){
    return stochastic;
}

Search::Node* Search::StochasticNode::get_parent(){
    return parent;
}

void Search::StochasticNode::set_score(double score_fn(Search::Node*)){
    if(!terminal){
        score = score_fn(this);
    }
    // if terminal, score should have been instantiated on creation and never change.
}

void Search::StochasticNode::update(double reward,double score_fn(Search::Node*)){
    TotalReward+=reward;
    N_visits+=1;

    set_score(score_fn);
}

void Search::StochasticNode::backprop(double reward,double score_fn(Search::Node*)){
    update(reward,score_fn);
    if(parent){
        // If parent isn't nullptr, tell the parent to backprop
        parent -> backprop(reward,score_fn);
    }
}

void Search::StochasticNode::addChild(Board::Board& game_board, Actions::Action* _action,GameLogic::Game& game_logic){
    _action -> execute(game_board);
    if(game_logic.is_stochastic(game_board)){
        children.push_back(new StochasticNode(this,_action,&game_board,game_logic));
    } else {
        children.push_back(new DeterministicNode(this,_action,&game_board,game_logic));
    }
}

Search::Node* Search::StochasticNode::getChild(int child){
    return children[child];
}

Search::GameTree::GameTree(GameLogic::Game& _game_logic): game_logic(_game_logic){
    // Get a copy of the board under consideration
    Board::Board state = game_logic.board_copy();

    // A game tree is only ever instantiated for an agent at a decision point (so deterministic node)
    root = new Search::DeterministicNode(nullptr,nullptr,&state,game_logic);
}

Search::KSampleGameTree::KSampleGameTree(GameLogic::Game& _game_logic,int _samples_per_stochasticity):GameTree(_game_logic){
    samples_per_stochasticity = _samples_per_stochasticity;
}

Search::Node* Search::KSampleGameTree::getBestLeaf(Board::Board& game_board){
    /*
    Idea is that this kind of tree will traverse through deterministic choices by evolving a copy of the current state with node `action`s, until it is returned a new node by some `best_child` method.

    If this results in a node (and state) that requires player input, then we just return the new deterministic node

    If this results in a stochastic node (ie state that requires game logic to advance), then:
        * use game logic to generate samples_per_stochasticity stochastic actions (with replacement) and create immediate children with each action
        * For each of samples_per_stochasticity children, copy the current state and make StochasticNode children in a chain succeeding that child until another decision node is reached, storing the stochasticity drawn in each node
    
    */

    // Always start by asking the root for a best child
    // We're assuming this is only ever a deterministic node
    // (advances the game_board in place)
    Search::Node* best_choice = root -> best_child(game_board,game_logic);

    // Then traverse the tree until it's forced to instantiate a new node
    // Goal is to end up at the next decision node
    while(!best_choice -> terminal && (best_choice -> N_visits > 0)){
        //  best_child() methods always advance given board state in-place
        // eventually, though, it should have to make a new node (N_visits=0) or get to the end of a game (terminal=true)
        best_choice = best_choice -> best_child(game_board,game_logic);
    }

    // If this is stochastic (and not the end of the game), then by construction it means a DeterministicNode made it
    // It's up to this tree, right now, to choose how to instantiate children for the stochasticity.
    // In this tree I:
    //  (1) instantiate samples_per_stochasticity children of this node
    //  (2) make a chain of nodes one at a time under each child until the game reaches another decision node
    //  (3) advance the game_board through one of the children chains before returning the new decision node to the agent
    if(best_choice -> stochastic && !best_choice -> terminal){
        for(int child=0;child<samples_per_stochasticity;child++){
            // Copy the board
            Board::Board board_copy = game_board;

            // generate an action for this copy of the board without advancing it
            Actions::Action* new_action = game_logic.get_stochastic_action(board_copy);
            best_choice -> addChild(board_copy,new_action,game_logic);

            // get the newly made child at the top of a chain of stochasticities
            Search::Node* child_link = best_choice -> getChild(child);
            while(game_logic.is_stochastic(board_copy)){
                // get a new action in this board state without advancing it
                Actions::Action* new_action = game_logic.get_stochastic_action(board_copy);
                // add a link to this child
                child_link -> addChild(board_copy,new_action,game_logic);
                // repeat
                child_link = child_link -> getChild(0);
            }
        }

        // finish by traversing one of the stochastic chains and altering the board in-place
        best_choice = best_choice -> best_child(game_board,game_logic);
    }
    return best_choice;
}

Actions::Action* Search::KSampleGameTree::bestAction(){

    // Copy initial state from the game logic
    Board::Board state = game_logic.board_copy();

    Search::Node* best_choice = root -> best_child(state,game_logic);

    return best_choice -> get_action();
}