#ifndef SEARCH_H
#define SEARCH_H

#include <queue>
#include <vector>

#include "../../game_files/Actions.h"
#include "../../game_files/GameLogic.h"

namespace Search
{

    class Node{

        // children have to be implemented on child classes
        // (it makes a difference whether we're talking about stochastic or deterministic nodes

    public:
        Node(Node* _parent,Actions::Action* _action);
        virtual ~Node(){};
        // action applied after parent action in order to derive a new, unique state
        // For deterministic nodes this is clear enough
        // For stochastic nodes it indicates a sampled stochastic action drawn by the game logic
        //      (exactly how to traverse stochastic nodes is up to StochasticNode implementation)
        Actions::Action* action;

        bool stochastic;
        bool terminal;

        int N_visits=0; // # of times tree search has gone through this node
        double TotalReward=0; // Amount of total reward realized on passes through this node during search

        // All nodes will have a score (for the sake of always being able to rank children)
        // For stochastic nodes it'll represent an expectation of reward upon traversal (via implicit calculation)
        double score =0;

        // parent
        Node* parent = nullptr;

        virtual Node* get_parent()=0;

        // any node should be able to retrieve a best child
        virtual Node* best_child(Board::Board& state,GameLogic::Game& game_logic)=0;

        // whether stochastic or not should be able to pass rewards back up through the tree
        virtual void backprop(double reward,double score_fn(Node*))=0; 

        // Update its score with a given reward (called in backprop)
        virtual void update(double reward,double score_fn(Node*))=0;

        // Be able to return an action used to get to the node.
        virtual Actions::Action* get_action()=0;

        virtual int n_children()=0;

        // Be able to add a child with an action curated from the outside
        // (for stochastic childclass only)
        virtual void addChild(Board::Board& game_board,Actions::Action* action,GameLogic::Game& game_logic)=0;

        // Be able to access `child`th child of a stochastic node.
        // No meaning for a deterministic node
        virtual Node* getChild(int child)=0; 

        // Not even sure if this is used when the priority queue is pointers
        bool operator < (const Node& rhs){
            // Only care about the score
            // This operator should only be used in DeterministicNode priority_queue of children
            // In this context the comparison of score is the only relevant consideration
            // More generally you'd want to care that the actions weren't the same, e.g.
            return score<rhs.score;
        }
    };

    // General pointer comparison by calling overloaded < on object at pointer location
    // When I tried to specify specifically for Nodes it'd complain that Node wasn't defined
    // Then when moved after, `children` declaration complained that comparison wasn't defined...
    struct pCompare{
        template <typename T> 
        bool operator()(T* a, T* b){
            return (*a)<(*b);
        }
    };

    class DeterministicNode: public Node{

        std::vector<Actions::Action*> action_queue; // The actions returned by gamelogic as being applicable in the state attached to this node.

        std::priority_queue<Node*,std::vector<Node*>,pCompare> children;

    public:
        // Always instantiate for a specific state
        // State should be the state being "rolled down" from the root and accumulating an action.execute() change at each node
        DeterministicNode(Node* _parent,Actions::Action* _action, Board::Board* _board_state,GameLogic::Game& game_logic);
        ~DeterministicNode(){
            // Delete any actions remaining in the queue
            while(!action_queue.empty()){
                delete action_queue.back();
                action_queue.pop_back();
            }
            // Delete any children (aka filicide)
            while(!children.empty()){
                Node* child = children.top();
                children.pop();

                delete child;
            }
            // delete the action pinned to this node
            delete action;
        }

        // Score of the node. Updated on backprop()
        double score=0; 

        Node* get_parent();

        // Score of the node
        void set_score(double score_fn(Node*));

        double get_TotalReward(); // for getting a score when updating according to a scoring function

        // Update its score with a given reward (called in backprop)
        void update(double reward,double score_fn(Node*));

        void backprop(double reward,double score_fn(Node*)); // for any state heuristics eventually  used
        void backprop(); // for when a node is terminal

        // Be able to return an action used to get to the node.
        Actions::Action* get_action();

        bool is_terminal(); // whether the state is terminal (win/loss/broken)
        bool is_stochastic(); // whether the action attached to this node results in a state from which a game-logic stochastic transition is required

        // Use the score that' been defined and iterated during tree growth to pull the best child
        // If action queue isn't empty yet, always return a Node for the new action
        Node* best_child(Board::Board& state,GameLogic::Game& game_logic);

        int n_children();

        void addChild(Board::Board& game_board,Actions::Action* action,GameLogic::Game& game_logic);
        Node* getChild(int child); 
    };

    class StochasticNode: public Node{

        // Unlike deterministic nodes, there's no notion that children need to be ranked
        std::vector<Node*> children;

    public:
        // Always instantiate given a specific state
        // State should be the state being "rolled down" from the root and accumulating an action.execute() change at each node
        StochasticNode(Node* _parent,Actions::Action* _action, Board::Board* _board_state,GameLogic::Game& game_logic);
        ~StochasticNode(){
            // delete the action pinned to this node
            delete action;

            // delete each child
            for(Node* child: children){
                delete child;
            }
            // empty the vector of now meaningless pointers (not sure if necessary)
            children.clear();
        }

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

        // Use the score that' been defined and iterated during tree growth to pull the best child
        // If action queue isn't empty yet, always return a Node for the new action
        Node* best_child(Board::Board& state,GameLogic::Game& game_logic);

        int n_children();

        void addChild(Board::Board& game_board,Actions::Action* action,GameLogic::Game& game_logic);
        Node* getChild(int child); 
    };

    // Virtual GameTree class
    // In general, Game trees will try saving memory by only memorizing states at nodes that are the result of a stochastic transition.
    // A game tree should otherwise just make an initial copy of the root state and transition it in-place using actions on each node
    class GameTree{
    public:
        GameTree(GameLogic::Game& _game_logic);
        virtual ~GameTree(){};
        Node* root;
        GameLogic::Game& game_logic; // reference to game logic used by agent

        // Use scores on existing nodes to get to the "best" leaf
        // (which may be a previously untried action on a "best" distal child)
        // (Equivalent of "TreePolicy" to return a leaf for exploration)
        virtual Node* getBestLeaf(Board::Board& game_board)=0;

        // Return the best root-child (action to take by the agent)
        virtual Actions::Action* bestAction()=0; 
    };

    // A tree that will sample K complete stochastic transitions upon arriving at a stochastic node,
    // and use one of K at random upon each subsequent traversal
    //      (results in much slower growth of tree depth when exploring below stochasticity with K>1)
    // More specifically, will randomly generate K (probably but maybe not different) sequences of card draws, each of which terminates at an agent decision node
    class KSampleGameTree: public GameTree{
        int samples_per_stochasticity;
    public:
        KSampleGameTree(GameLogic::Game& _game_logic, int _samples_per_stochasticity);
        ~KSampleGameTree(){
            // Send cascade of deletes down the tree
            delete root;
        }
        Node* getBestLeaf(Board::Board& game_board);

        Actions::Action* bestAction();
    };

    // UCB1 score
    double UCB1Score(Node* node);
}

#endif