#ifndef SEARCH_H
#define SEARCH_H

#include <queue>
#include <vector>

#include "../../game_files/Actions.h"
#include "../../game_files/GameLogic.h"

namespace Search
{
    // General pointer comparison by calling overloaded < on object at pointer location
    // When I tried to specify specifically for Nodes it'd complain that Node wasn't defined
    // Then when moved after, `children` declaration complained that comparison wasn't defined...
    struct pCompare{
        template <typename T> 
        bool operator()(T* a, T* b){
            return (*a)<(*b);
        }
    };

    class Node{
    Node* parent = nullptr;

    std::priority_queue<Node*,std::vector<Node*>,pCompare> children;

    int N_visits=0; // # of times tree search has gone through this node
    double TotalReward=0; // Amount of total reward realized on passes through this node during search

    bool terminal=false; // Whether or not the state it holds is terminal (set at instantiation)
    bool stochastic=false; // Whether or not the next state-modification required is from the game logic (true - drawing cards) or from the player (false - deterministic action)

    Actions::Action* action; // The action used to arrive at the state of this node
    std::vector<Actions::Action*> action_queue; // The actions returned by gamelogic as being applicable in the state attached to this node.

    Board::Board* board_state=nullptr;

    public:
        // Always instantiate for a specific state
        Node(Node* _parent,Actions::Action* _action, Board::Board* _board_state,GameLogic::Game& game_logic, bool initialize=false);

        // Instantiate the action queue using the attached state
        void instantiate_action_queue(GameLogic::Game& game_logic);

        // Score of the node. Updated on backprop()
        double score=0; 

        Node* get_parent();

        // Score of the node
        void set_score(double score_fn(Node*));

        // Whether or not the node has been visited
        bool visited();
        int n_visited(); // for returning how often its been visited (e.g. for UCT exploration)

        double get_TotalReward(); // for getting a score when updating according to a scoring function

        // Update its score with a given reward (called in backprop)
        void update(double reward,double score_fn(Node*));

        void backprop(double reward,double score_fn(Node*)); // for any state heuristics eventually  used
        void backprop(); // for when a node is terminal

        // Be able to return an action used to get to the node.
        Actions::Action* get_action();

        bool is_terminal(); // whether the state is terminal (win/loss/broken)
        bool is_stochastic(); // whether the action attached to this node results in a state from which a game-logic stochastic transition is required
        void deleteState(); // free the state pointer on the current state;

        // Use the score that' been defined and iterated during tree growth to pull the best child
        // If action queue isn't empty yet, always return a Node for the new action
        Node* best_child(Board::Board& state,GameLogic::Game& game_logic);

        // return a copy of the node state
        // (for use when the result of a stochastic transition is pinned on a node)
        Board::Board get_state(); 
        
        // return pointer to pinned board state
        Board::Board* get_state_ptr();

        // Not even sure if this is used when the priority queue is pointers
        bool operator < (const Node& rhs){
            // Compare representation of the actions. If the same, then < never true. Otherwise depends on score.
            if(action -> repr() != rhs.action -> repr()){
                return score<rhs.score;
            } else {
                return false;
            }
        }
    };

    // Virtual GameTree class
    // In general, Game trees will try saving memory by only memorizing states at nodes that are the result of a stochastic transition.
    // A game tree should otherwise just make an initial copy of the root state and transition it in-place using actions on each node
    class GameTree{
    public:
        GameTree(GameLogic::Game& _game_logic);

        Node* root;
        GameLogic::Game& game_logic; // reference to game logic used by agent

        // Use scores on existing nodes to get to the "best" leaf
        // (which may be a previously untried action on a "best" distal child)
        // (Equivalent of "TreePolicy" to return a leaf for exploration)
        virtual Node* getBestLeaf()=0;

        // Return the best root-child (action to take by the agent)
        virtual Actions::Action* bestAction()=0; 

        // This is an entry-point for the agent to be able to save memory by removing leaf states it's already rolled out
        virtual void deleteState(Node* node)=0; 
    };

    // A tree that will make a single stochastic transition upon arriving at a stochastic node,
    // and use the result for every subsequent visitation of that action
    // (e.g. A players last action is used to move to Washington. This game tree will then draw cards for the player etc.
    // The exact cards drawn will be used for every subsequent visitation of the action "Move to washington")
    class SingleSampleGameTree: public GameTree{
    public:
        SingleSampleGameTree(GameLogic::Game& _game_logic);

        Node* getBestLeaf();

        Actions::Action* bestAction();

        void deleteState(Node* node);
    };

    // UCB1 score
    double UCB1Score(Node* node);
}
#endif