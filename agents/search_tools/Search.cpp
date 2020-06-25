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

    // increment stochasticities traversed if you came from a stochastic parent
    if(parent){
        if(parent -> stochastic){
            stochasticities_traversed = parent -> stochasticities_traversed+1;
        } else {
            stochasticities_traversed = parent -> stochasticities_traversed;
        }
    }
}

Search::DeterministicNode::DeterministicNode(Search::Node* _parent,Actions::Action* _action, Board::Board* _board_state,GameLogic::Game& game_logic):
    Node(_parent,_action){
    // An instantiated node is always labeled with terminal = true if the state to which it's associated is terminal
    if(game_logic.is_terminal(*_board_state)){
        terminal = true;
        action_queue = {};

        // set the score here to 0/1. Tree evaluation should *always* see the true reward of a terminal state added into the tree
        score= (double) game_logic.reward(*_board_state);
    } else {
        terminal=false;
        action_queue = game_logic.list_actions(*_board_state);
    }

    if(parent){
        depth = parent->depth+1;
    } else{
        // If the root node, then depth=0
        depth=0;
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

double Search::DeterministicNode::update(double score_fn(Search::Node*)){
    set_score(score_fn);
    return score;
}

void Search::DeterministicNode::backprop(double reward){
    if(!terminal){
        TotalReward+=reward;
    }
    N_visits+=1;
    if(parent){
        // backpropogate either fixed score if terminal, or reward if not
        parent -> backprop(terminal ? score : reward);
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

Search::Node* Search::DeterministicNode::best_child(Board::Board& state,GameLogic::Game& game_logic,double score_fn(Search::Node*)){
    if(!terminal){
        if(action_queue.empty()){
            // If there are no untried actions, always return the best child as per score that's been being updated
            
            // Necessary to check scores by force though
            // I do a by-force max check rather than sorting the vector because it's costly
            // sort is O(N log N) and so potentially worse than this, which in addition to the fact that my compiler ignores std::sort means I don't use that
            double best_score=-1;
            Search::Node* best_child = nullptr;

            for(Search::Node* child: children){
                double this_score = child -> update(score_fn);
                if(this_score > best_score){
                    best_score = this_score;
                    best_child = child;
                }
            }

            // advance the state in-place
            best_child -> get_action() -> execute(state);

            // Return the best child after having updating the queue and advanced the state
            return best_child;
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
            children.push_back(new_node);

            // And return it as the "best child"
            return new_node;
        }
    }
    // should break if a terminal node is being asked for children
    return nullptr;
}

void Search::DeterministicNode::addChild(Board::Board& game_board, Actions::Action* action,GameLogic::Game& game_logic){};

void Search::DeterministicNode::addNullChild(){};

Search::Node* Search::DeterministicNode::getChild(int child){return children[child];};

bool Search::DeterministicNode::converged(int num_visits){
    return N_visits>=num_visits && action_queue.empty();
}

void Search::DeterministicNode::setChild(int child,Actions::Action* _action, Board::Board* _board_state,GameLogic::Game& game_logic){};

int Search::StochasticNode::n_children(){
    return children.size();
};

Search::StochasticNode::StochasticNode(Search::Node* _parent,Actions::Action* _action, Board::Board* _board_state,GameLogic::Game& game_logic):
    Node(_parent,_action){
        stochastic = true;

        if(game_logic.is_terminal(*_board_state)){
            terminal = true;

            // set the score here to 0/1. Tree evaluation should *always* see the true reward of a terminal state added into the tree
            score = (double) game_logic.reward(*_board_state);
        }

        if(!parent -> stochastic){
            // If parent is deterministic, then increase depth by 1 (a choice was made to put the game at a chance node)
            depth = parent->depth+1;
        } else{
            // If parent is stochastic, then keep same depth (this node is part of the resolution of stochasticity, and incrementing depth is meaningless)
            depth = parent -> depth;
        }

        // I'm choosing to implement stochastic nodes with no children on the constructor. 
        // It's up to the tree that's using such nodes to decide what to do with them.
        children = {};
    }

Search::Node* Search::StochasticNode::best_child(Board::Board& state,GameLogic::Game& game_logic,double score_fn(Search::Node*)){
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
                best_child = best_child -> best_child(state,game_logic,score_fn);
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

bool Search::StochasticNode::converged(int num_visits){
    // Idea: Only say it's converged if N_visits>=num_visits AND all children exist (all determinizations used)
    return N_visits>=num_visits;
}

void Search::StochasticNode::set_score(double score_fn(Search::Node*)){
    if(!terminal){
        score = score_fn(this);
    } 
    // If the node is terminal, keep the original game logic assignment of score as 0/1
}

double Search::StochasticNode::update(double score_fn(Search::Node*)){
    set_score(score_fn);
    return score;
}

void Search::StochasticNode::backprop(double reward){
    if(!terminal){
        TotalReward+=reward;
    }
    N_visits+=1;
    if(parent){
        // If parent isn't nullptr, tell the parent to backprop with either set score on this terminal node, or observed rollout reward for nonterminal
        parent -> backprop(terminal ? score : reward);
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

void Search::StochasticNode::addNullChild(){
    children.push_back(nullptr);
}

Search::Node* Search::StochasticNode::getChild(int child){
    return children[child];
}

void Search::StochasticNode::setChild(int child,Actions::Action* _action, Board::Board* _board_state,GameLogic::Game& game_logic){
    _action -> execute(*_board_state);
    if(game_logic.is_stochastic(*_board_state)){
        children[child] = new Search::StochasticNode(this,_action,_board_state,game_logic);
    } else {
        children[child] = new Search::DeterministicNode(this,_action,_board_state,game_logic);
    }
}

Search::GameTree::GameTree(GameLogic::Game& _game_logic): game_logic(_game_logic){
    // Get a copy of the board under consideration
    Board::Board state = game_logic.board_copy();

    // A game tree is only ever instantiated for an agent at a decision point (so deterministic node)
    root = new Search::DeterministicNode(nullptr,nullptr,&state,game_logic);
}

Search::KDeterminizedGameTree::KDeterminizedGameTree(GameLogic::Game& _game_logic,int _samples_per_stochasticity):GameTree(_game_logic){
    samples_per_stochasticity = _samples_per_stochasticity;
    for(int det=0;det<samples_per_stochasticity;det++){
        // Initialize the determinization queue with empty determinizations
        determinization_queue.push_back({});
    }
}

Search::Node* Search::KDeterminizedGameTree::getBestLeaf(Board::Board& game_board,double score_fn(Search::Node*)){
    /*

    ---> Tree traversed this way --->
            _____ * ______ * ________ x
          /    
         /
    x-- * ------ * ------- * ------- x    <- This is an artistic interpretation (with license) of what I'm trying to achieve. Here "*" are stochastic nodes, and "x" are deterministic nodes
         \                                      This shows a stochastic node in a "3-determinized" tree. The first stochastic node encountered branches 3 times, one for each determinization
          \______ * _______ x                   Subsequent stochastic nodes each have one child until they hit a deterministic node
       
    This tree traverses the tree evolving `game_board` in place with actions on traversed nodes.
    */

    // Always start by asking the root for a best child
    // We're assuming this is only ever a deterministic node
    // (this request advances the game_board in place)
    Search::Node* best_choice = root -> best_child(game_board,game_logic,score_fn);

    // Choose which determinization we'll use for all the stochasticities in this traversal
    int determinization = rand() % samples_per_stochasticity;

    // Then traverse the tree until it's forced to instantiate a new node
    // Goal is to end up at the next node thats an unvisited decision node or terminal node
    while(!best_choice -> terminal && (best_choice -> N_visits > 0)){
        // If this is a deterministic node
        if(!best_choice -> stochastic){
            // then select the best performing child node
            best_choice = best_choice -> best_child(game_board,game_logic,score_fn);
        } else {
            // otherwise follow (perhaps create-and-follow) the `determinization`th child of the stochastic node down to it's deterministic successor
            best_choice = GetDeterministicChild(best_choice,game_board,determinization);
        }
    }

    // Return either the deterministic node, or the `determinization`th deterministic successor of a deterministic successor
    return GetDeterministicChild(best_choice,game_board,determinization);
}

Search::Node* Search::KDeterminizedGameTree::bestRootChild(double score_fn(Search::Node*)){
    // Copy initial state from the game logic
    Board::Board state = game_logic.board_copy();

    Search::Node* best_choice = root -> best_child(state,game_logic,score_fn);

    return best_choice;
}

Actions::Action* Search::KDeterminizedGameTree::GetOrCreateAction(Search::Node* node_for_expansion, Board::Board& board_copy, int determinization){
    Actions::Action* new_action = nullptr;
    if(determinization_queue[determinization].size()>node_for_expansion -> stochasticities_traversed){
        // If there are at least as many determinizations as we need to get the first action, then use that;
        new_action =  determinization_queue[determinization][node_for_expansion -> stochasticities_traversed];
        // (k stochasticities have been traversed -> We need to use the (k+1)th to advance to the next node)
        // (and (k+1)th entry = index k)
    } else {
        // If there aren't as many as we need, then by construction (since this is a single new node on the tree) we *should* only require one new action
        new_action = game_logic.get_stochastic_action(board_copy);
        // We put this new action on the `determinization`th determinization queue
        determinization_queue[determinization].push_back(new_action);
    }
    return new_action;
}

Search::Node* Search::KDeterminizedGameTree::GetDeterministicChild(Search::Node* node, Board::Board& game_board, int determinization,bool on_chain){
    if(!node -> stochastic){
        // always return a deterministic node immediately, 
        return node;
    } else {
        // If it's not, check to see whether any children have been instantiated before
        if(node -> n_children()==0){
            if(on_chain){
                // If within the child-after-child chain under an already-branched stochasticity, add a single successor
                node -> addNullChild();
            } else {
                // Otherwise samples_per_stochasticity children, one for each determinization
                for(int _=0;_<samples_per_stochasticity;_++){
                    node -> addNullChild();
                }
            }
        }
    }

    // If the appropriate child is already defined...
    if(node -> getChild(on_chain ? 0 : determinization)){
        // Then get it, advance the state using it's action, and return the recursive call
        Search::Node* child = node -> getChild(on_chain ? 0 : determinization);
        Actions::Action* action = child -> get_action();
        action -> execute(game_board);

        return GetDeterministicChild(child,game_board,determinization,true);
    } else {
        // Otherwise, if the child isn't defined yet,...

        // Get-or-create an action with which to define a successor. `node` is used to determine how far into the determinization to look
        Actions::Action* new_action = GetOrCreateAction(node,game_board,determinization);

        // set a child and advance the board in-place
        node -> setChild(on_chain ? 0 : determinization,new_action,&game_board,game_logic);

        // Get that child
        Search::Node* child = node -> getChild(on_chain ? 0 : determinization);

        // recursively call the function
        return GetDeterministicChild(child,game_board,determinization,true);
    }
}