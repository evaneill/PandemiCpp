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
    value+= ((double) node -> get_TotalReward() / (double) node ->n_visited()); // average reward

    int n_parent_visits;
    if(!node -> get_parent()){
        // If no parent (its the root), they're the same value and this calculation won't matter anyway
        n_parent_visits = node -> n_visited();
    } else {
        n_parent_visits = node -> get_parent() -> n_visited();
    }

    // sqrt(2*ln(# parent visits)/(# child visits))
    value+=std::pow(2*std::log((double) n_parent_visits)/(double) node -> n_visited(),.5);

    return value;
}

Search::Node::Node(Search::Node* _parent,Actions::Action* _action, Board::Board* _board_state,GameLogic::Game& game_logic, bool initialize){
    // An instantiated node is always labeled with terminal = true if the state to which it's associated is terminal
    if(_board_state -> is_terminal()){
        terminal = true;
        action_queue = {};
    } else {
        // And always labeled stochastic if it requires stochastic transition
        if(game_logic.is_stochastic(*_board_state)){
            stochastic = true;
        }
    }

    // I'm only going to instantiate an action queue for the root node (initialize=true)
    // I'm going to force all of GameTree implementations to decide on their own means to create action queues for non-root nodes
    // (This is because different trees might treat stochastic nodes differently)
    if(initialize){
        action_queue = game_logic.list_actions(*_board_state);
    } else {
        // Root doesn't need a copy of the board state on it - it'll never get returned to the agent for a rollout!
        // But any other node instantiated for the agent will need one
        board_state = _board_state;
    }

    // Always initialize empty children on a new node
    children = {};

    // Always has an action and parent, even if parent/action is nullptr
    action = _action;
    parent = _parent;
}

void Search::Node::set_score(double score_fn(Search::Node*)){
    if(!terminal){
        score = score_fn(this);
    }
    // if terminal, score should have been instantiated on creation and never change.
}

bool Search::Node::visited(){
    return N_visits>0;
}

int Search::Node::n_visited(){
    return N_visits;
}

double Search::Node::get_TotalReward(){
    return TotalReward;
}

void Search::Node::update(double reward,double score_fn(Search::Node*)){
    TotalReward+=reward;
    N_visits+=1;

    set_score(score_fn);
    if(!stochastic){
        // always make sure to set board_state=nullptr for non-stochastic states
        deleteState();
    }
}

void Search::Node::backprop(double reward,double score_fn(Search::Node*)){
    update(reward,score_fn);
    if(parent){
        // If parent isn't nullptr, tell the parent to backprop
        parent -> backprop(reward,score_fn);
    }
}

Actions::Action* Search::Node::get_action(){
    return action;
}

bool Search::Node::is_terminal(){
    return terminal;
}

bool Search::Node::is_stochastic(){
    return stochastic;
}

void Search::Node::deleteState(){
    board_state = nullptr;
}

Board::Board Search::Node::get_state(){
    return *board_state;
}

Board::Board* Search::Node::get_state_ptr(){
    return board_state;
}

Search::Node* Search::Node::get_parent(){
    return parent;
}

void Search::Node::instantiate_action_queue(GameLogic::Game& game_logic){
    action_queue = game_logic.list_actions(*board_state);
}

Search::Node* Search::Node::best_child(Board::Board& state,GameLogic::Game& game_logic){
    if(action_queue.empty()){
        // If there are no untried actions, always return the best child as per score that's been being updated
        
        // Necessary to force the queue to update tho
        Search::Node* was_best_before = children.top();
        children.pop();
        children.push(was_best_before);
        
        // Return the best child after updating the queue
        return children.top();
    } else {
        // Otherwise pull an action out of the stack
        Actions::Action* new_action = action_queue.back();
        new_action -> execute(state); // alter the state in place
        // Delete the action from the stack
        action_queue.pop_back();
        // And define a new node with this action
        Search::Node* new_node = new Node(this,new_action,&state,game_logic);
        // And insert it into children for later
        children.push(new_node);
        // And return it as the "best child"
        return new_node;
    }
}

Search::GameTree::GameTree(GameLogic::Game& _game_logic): game_logic(_game_logic){
    // Get a copy of the board under consideration
    Board::Board state = game_logic.board_copy();

    root = new Search::Node(nullptr,nullptr,&state,game_logic,true);
}

Search::SingleSampleGameTree::SingleSampleGameTree(GameLogic::Game& _game_logic):GameTree(_game_logic){};

Search::Node* Search::SingleSampleGameTree::getBestLeaf(){

    // Copy initial state from the game logic
    Board::Board state = game_logic.board_copy();

    // Start tree policy by getting the best child of the root.
    Search::Node* best_choice = root -> best_child(state,game_logic);

    // Continue traversing down through nodes.
    // Only stop when you reach a previously unvisited (i.e. newly instantiated) node or a terminal node
    while(best_choice -> visited() && ! best_choice -> is_terminal()){
        
        // If the best child node is stochastic...
        if(best_choice -> is_stochastic()){
            // make state = a copy of the state on the node (representing the result of the stochasticity seen the first time this was visited)
            // (implicitly toss out the copy we were using before)
            state = best_choice -> get_state();
        } else {
            // Otherwise use the node action to evolve the state in-place
            best_choice -> get_action() ->execute(state);
        }

        // Then get the best child node of _this_ child
        best_choice = best_choice -> best_child(state,game_logic);
    }

    // If the returned node requires a stochastic transition, evolve the state in-place.
    // All subsequent visitations of this node will use this state rather than re-sampling the stochatsticity
    // (Therefore "single sample Game Tree") 

    // If it's terminal, don't alter the node. 
    if(!best_choice->is_terminal()){
        if(best_choice -> is_stochastic()){
            // will advance the node state stochastically in-place, THEN attach available actions
            game_logic.nonplayer_actions(*(best_choice->get_state_ptr()));
        }
        best_choice -> instantiate_action_queue(game_logic);
    }
    return best_choice;
}

Actions::Action* Search::SingleSampleGameTree::bestAction(){

    // Copy initial state from the game logic
    Board::Board state = game_logic.board_copy();

    Search::Node* best_choice = root -> best_child(state,game_logic);

    return best_choice -> get_action();
}

void Search::SingleSampleGameTree::deleteState(Search::Node* node){
    // This type of game tree will never delete the state on a stochastic node
    if(!node -> is_stochastic()){

        // If it's not a stochastic node we always want to save memory by evolving a state copy in-place down the game tree
        node -> deleteState();
    }
}