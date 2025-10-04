#ifndef ACTIONS_H
#define ACTIONS_H

#include <string>
#include <vector>
#include <set>

#include "Board.h"
#include "Players.h"
#include "Map.h"
#include "Decks.h"

namespace Actions
{
    class Action{
        // To be constructed and returned, then called with execute();
    public:
        // E.g. "MOVE"
        std::string movetype;
        virtual ~Action(){};

        // Act on the referenced active_board according to child class logic
        virtual void execute(Board::Board& new_board)=0;
        
        // String representation for logging/debugging
        virtual std::string repr()=0;
    };

    /* Below are all the actions, each of which has
        (1) A reference to the active game board
        (2) Attributes that help define it's exact effect
        (3) An execute() method that operates on the active board
        (4) a repr() method that can be called to get a string representation
    It's important to note that it's up to the <Board> to decide which groups of actions are legal before asking for them.
    
    At this point actions are intentionally very NAIVE. For example:
        Direct flight/charter flight allow a player to discard a card to "fly" to the city they're currently in.

    Actions are:
        Player actions:
            <Move> between adjacent cities
            <DirectFlight> by discarding a held city card
            <CharterFlight> by discarding the card of the city you're in to move anywhere.
            <ShuttleFlight> by traveling between research stations
            <OperationsExpertFlight> (for OE only) by discarding a card at a research station to go anywhere, once per turn

            <Build> a research station 

            <Treat> disease on a city
            <Cure> disease at a city

            <Give> a card to another player
            <Take> a card from another player
        
        Event Card actions:
            <Airlift>: move any player to any location
            <GovernmentGrant>: build a research station anywhere there isn't already a station
            <QuietNight>: skip the next infect step

        NULL action:
            <DoNothing> You're allowed to do nothing!
    */

    // ===== Move ===== 
    class Move: public Action{
        // to incorporate dispatcher would either need a new action, or include player, from, and to here.
        int to;
    public:
        Move(int _to);
        ~Move();
        void execute(Board::Board& new_board); // To execute on a given board
        std::string repr(); // to yield a string representation for logging
    };

    // ===== DirectFlight ===== 
    class DirectFlight: public Action{
        int citycard_index; // card to discard
    public:
        DirectFlight(int city_index); // instantiate with index in hand (useful for randomization)
        ~DirectFlight();
        void execute(Board::Board& new_board); // To execute on a given board
        std::string repr(); // to yield a string representation for logging
    };

    // ===== CharterFlight =====
    class CharterFlight: public Action{
        int target_city;
    public:
        CharterFlight( int _target_city);
        ~CharterFlight();
        void execute(Board::Board& new_board); // To execute on a given board
        std::string repr(); // to yield a string representation for logging
    };

    // ===== ShuttleFlight =====
    class ShuttleFlight: public Action{
        int target_station_city_idx;
    public:
        ShuttleFlight( int _target_station);
        ~ShuttleFlight();
        void execute(Board::Board& new_board); // To execute on a given board
        std::string repr(); // to yield a string representation for logging
    };

    // ===== OperationsExpertFlight =====
    // (Like shuttle flight, but to ANY city)
    class OperationsExpertFlight: public Action{
        int target_city;
        int discard_card;
    public:
        OperationsExpertFlight(int _target_city,int _discard_card);
        ~OperationsExpertFlight();
        void execute(Board::Board& new_board); // To execute on a given board
        std::string repr(); // to yield a string representation for logging
    };

    // ===== Build =====
    class Build: public Action{
        // Will have to account for presence of existing stations.
        // This is the city index of the station to be removed
        int remove_station;
        int place_station;
    public:
        Build( int place_station,int _remove_station=-1);
        ~Build();
        void execute(Board::Board& new_board); // To execute on a given board
        std::string repr(); // to yield a string representation for logging
    }; 

    // ===== Treat ===== 
    class Treat: public Action{
        int color; // color to target
        int n_treated=-1; // To be set on execute() for repr() 
    public:
        Treat(int _color);
        ~Treat();
        void execute(Board::Board& new_board); // To execute on a given board
        std::string repr(); // to yield a string representation for logging
    };

    // ===== Cure ===== 
    class Cure: public Action{
        // when available b/c it's legal, there can only be one curable color (at least 4 cards of hand size 7 are needed)
        int color_cured =-1; // to be set on execute() for repr()
    public:
        Cure();
        ~Cure();
        void execute(Board::Board& new_board); // To execute on a given board
        std::string repr(); // to yield a string representation for logging
    };

    // ===== Give ===== 
    class Give: public Action{
        int other_player_turn;
        int card_to_give_city_idx;
    public:
        Give(int _other_player_turn, int _card_to_give_cityidx);
        ~Give();
        void execute(Board::Board& new_board); // To execute on a given board
        std::string repr(); // to yield a string representation for logging
    };

    // ===== Take ===== 
    class Take: public Action{
        int other_player;
        int card_to_take_city_idx;
    public:
        Take(int _other_player, int _card_to_take_city_idx);
        ~Take();
        void execute(Board::Board& new_board); // To execute on a given board
        std::string repr(); // to yield a string representation for logging
    };

    // ===== EVENT CARD ACTIONS =====
    // ===== Airlift ===== 
    class Airlift: public Action{
        int target_player; // turn order position of Player to move
        int target_city; // City to move active 
    public:
        Airlift(int _target_player,int _target_city);
        ~Airlift();
        void execute(Board::Board& new_board); // To execute on a given board
        std::string repr(); // to yield a string representation for logging
    };

    // ===== Government Grant ===== 
    class GovernmentGrant: public Action{
        int target_city;
        int remove_station;
    public:
        GovernmentGrant(int _target_city,int _remove_station=-1);
        ~GovernmentGrant();
        void execute(Board::Board& new_board); // To execute on a given board
        std::string repr(); // to yield a string representation for logging
    };

    // ===== Quiet Night ===== 
    class QuietNight: public Action{
    public:
        QuietNight();
        ~QuietNight();
        void execute(Board::Board& new_board); // To execute on a given board
        std::string repr(); // to yield a string representation for logging
    };

    // ===== Forced Discard Action 
    class ForcedDiscardAction: public Actions::Action{
    int discard_card_index;
    public:
        ForcedDiscardAction(int _discard_card_index);
        ~ForcedDiscardAction();
        void execute(Board::Board& new_board); // To execute on a given board
        std::string repr();
    };

    // ===== NULL ACTION ===== 
    // All other actions are coded so that this is UNIQUE;
    // ===== Do Nothing =====
    class DoNothing: public Action{
    public:
        DoNothing();
        ~DoNothing();
        void execute(Board::Board& new_board); // To execute on a given board
        std::string repr(); // to yield a string representation for logging
    };

    //This is an object that can instantiate any of a class of actions, using legal arguments.
    class ActionConstructor{
    public:
        // Get a representation
        virtual std::string get_movetype()=0;
        virtual ~ActionConstructor(){};

        // how many legal actions there are
        virtual int n_actions(Board::Board&  game_board)=0;
        virtual Actions::Action* random_action(Board::Board&  new_board)=0;
        virtual std::vector<Actions::Action*> all_actions(Board::Board&  new_board)=0;
        virtual bool legal(Board::Board&  new_board)=0;
    };

    // ===== PLAYER ACTION CONSTRUCTORS =====
    class MoveConstructor: public ActionConstructor{
        const std::string movetype = "MOVE"; // technically "Drive/Ferry"
        std::array<Actions::Move*,48> actionMap;
    public:
        MoveConstructor();
        ~MoveConstructor();

        // Get a representation
        std::string get_movetype();
        
        // how many legal actions there are
        int n_actions(Board::Board& game_board); 
        Actions::Action* random_action(Board::Board& game_board); // return a random action of thie movetype with legal arguments
        std::vector<Actions::Action*> all_actions(Board::Board& game_board); //  a vector of all legal actions for this movetype (E.g. instantiate a list of Action objects with all possible attributes)
        bool legal(Board::Board& game_board); // whether there is any legal use of this movetype
    };

    class DirectFlightConstructor: public ActionConstructor{
        const std::string movetype = "DIRECTFLIGHT";
        std::array<Actions::DirectFlight*,48> actionMap;
    public:
        DirectFlightConstructor();
        ~DirectFlightConstructor();

        // Get a representation
        std::string get_movetype();

        // how many legal actions there are
        int n_actions(Board::Board& game_board);
        Actions::Action* random_action(Board::Board& game_board); // return a random action of thie movetype with legal arguments
        std::vector<Actions::Action*> all_actions(Board::Board& game_board); //  a vector of all legal actions for this movetype (E.g. instantiate a list of Action objects with all possible attributes)
        bool legal(Board::Board& game_board); // whether there is any legal use of this movetype
    };

    class CharterFlightConstructor: public ActionConstructor{
        const std::string movetype = "CHARTERFLIGHT";
        // If available, always 47 actions: one for each city to fly to that's not the player's current one.
        std::array<Actions::CharterFlight*,48> actionMap;
    public:
        CharterFlightConstructor();
        ~CharterFlightConstructor();

        // Get a representation
        std::string get_movetype();

        // how many legal actions there are
        int n_actions(Board::Board& game_board);
        Actions::Action* random_action(Board::Board& game_board); // return a random action of thie movetype with legal arguments
        std::vector<Actions::Action*> all_actions(Board::Board& game_board); //  a vector of all legal actions for this movetype (E.g. instantiate a list of Action objects with all possible attributes)
        bool legal(Board::Board& game_board); // whether there is any legal use of this movetype
    };

    class ShuttleFlightConstructor: public ActionConstructor{
        const std::string movetype = "SHUTTLEFLIGHT";
        // If it's legal we can move to any of the other research stations
        std::array<Actions::ShuttleFlight*,48> actionMap;
    public:
        ShuttleFlightConstructor();
        ~ShuttleFlightConstructor();

        // Get a representation
        std::string get_movetype();

        // how many legal actions there are
        int n_actions(Board::Board& game_board);
        Actions::Action* random_action(Board::Board& game_board); // return a random action of thie movetype with legal arguments
        std::vector<Actions::Action*> all_actions(Board::Board& game_board); //  a vector of all legal actions for this movetype (E.g. instantiate a list of Action objects with all possible attributes)
        bool legal(Board::Board& game_board); // whether there is any legal use of this movetype
    };

    class OperationsExpertFlightConstructor: public ActionConstructor{
        const std::string movetype = "OPERATIONSEXPERTFLIGHT";
        // If it's legal we can move to any of the other research stations
        std::array<std::array<Actions::OperationsExpertFlight*,48>,48> actionMap;
    public:
        OperationsExpertFlightConstructor();
        ~OperationsExpertFlightConstructor();

        // Get a representation
        std::string get_movetype();

        // how many legal actions there are
        int n_actions(Board::Board& game_board);
        Actions::Action* random_action(Board::Board& game_board); // return a random action of thie movetype with legal arguments
        std::vector<Actions::Action*> all_actions(Board::Board& game_board); //  a vector of all legal actions for this movetype (E.g. instantiate a list of Action objects with all possible attributes)
        bool legal(Board::Board& game_board); // whether there is any legal use of this movetype
    };

    class BuildConstructor: public ActionConstructor{
        const std::string movetype = "BUILD";
        // If it's legal we can build a research station at the position of the current player.
        std::array<std::array<Actions::Build*,49>,48> actionMap;
    public:
        BuildConstructor();
        ~BuildConstructor();

        // Get a representation
        std::string get_movetype();

        // how many legal actions there are
        int n_actions(Board::Board& game_board);
        Actions::Action* random_action(Board::Board& game_board); // return a random action of thie movetype with legal arguments
        std::vector<Actions::Action*> all_actions(Board::Board& game_board); //  a vector of all legal actions for this movetype (E.g. instantiate a list of Action objects with all possible attributes)
        bool legal(Board::Board& game_board); // whether there is any legal use of this movetype
    };

    class TreatConstructor: public ActionConstructor{
        const std::string movetype = "TREAT";
        std::array<Actions::Treat*,4> actionMap;
    public:
        TreatConstructor();
        ~TreatConstructor();

        // Get a representation
        std::string get_movetype();

        // how many legal actions there are
        int n_actions(Board::Board& game_board);
        Actions::Action* random_action(Board::Board& game_board); // return a random action of thie movetype with legal arguments
        std::vector<Actions::Action*> all_actions(Board::Board& game_board); //  a vector of all legal actions for this movetype (E.g. instantiate a list of Action objects with all possible attributes)
        bool legal(Board::Board& game_board); // whether there is any legal use of this movetype
    };

    class CureConstructor: public ActionConstructor{
        const std::string movetype = "CURE";
        Actions::Cure* cureAction;
    public:
        CureConstructor();
        ~CureConstructor();

        // Get a representation
        std::string get_movetype();

        // how many legal actions there are
        int n_actions(Board::Board& game_board);
        Actions::Action* random_action(Board::Board& game_board); // return a random action of thie movetype with legal arguments
        std::vector<Actions::Action*> all_actions(Board::Board& game_board); //  a vector of all legal actions for this movetype (E.g. instantiate a list of Action objects with all possible attributes)
        bool legal(Board::Board& game_board); // whether there is any legal use of this movetype
    };

    class GiveConstructor: public ActionConstructor{
        const std::string movetype = "GIVE";
        std::array<std::array<Actions::Give*,48>,5> actionMap;
    public:
        GiveConstructor();
        ~GiveConstructor();

        // Get a representation
        std::string get_movetype();

        // how many legal actions there are
        int n_actions(Board::Board& game_board);
        Actions::Action* random_action(Board::Board& game_board); // return a random action of thie movetype with legal arguments
        std::vector<Actions::Action*> all_actions(Board::Board& game_board); //  a vector of all legal actions for this movetype (E.g. instantiate a list of Action objects with all possible attributes)
        bool legal(Board::Board& game_board); // whether there is any legal use of this movetype
    };

    class TakeConstructor: public ActionConstructor{
        const std::string movetype = "TAKE";
        std::array<std::array<Actions::Take*,48>,5> actionMap;
    public:
        TakeConstructor();
        ~TakeConstructor();

        // Get a representation
        std::string get_movetype();

        // how many legal actions there are
        int n_actions(Board::Board& game_board);
        Actions::Action* random_action(Board::Board& game_board); // return a random action of thie movetype with legal arguments
        std::vector<Actions::Action*> all_actions(Board::Board& game_board); //  a vector of all legal actions for this movetype (E.g. instantiate a list of Action objects with all possible attributes)
        bool legal(Board::Board& game_board); // whether there is any legal use of this movetype
    };

    // ===== EVENT CARD ACTION CONSTRUCTORS =====

    
    class AirliftConstructor: public ActionConstructor{
        const std::string movetype = "AIRLIFT";
        std::array<std::array<Actions::Airlift*,48>,5> actionMap;
    public:
        AirliftConstructor();
        ~AirliftConstructor();

        // Get a representation
        std::string get_movetype();

        // how many legal actions there are
        int n_actions(Board::Board& game_board);
        Actions::Action* random_action(Board::Board& game_board); // return a random action of thie movetype with legal arguments
        std::vector<Actions::Action*> all_actions(Board::Board& game_board); //  a vector of all legal actions for this movetype (E.g. instantiate a list of Action objects with all possible attributes)
        bool legal(Board::Board& game_board); // whether there is any legal use of this movetype
    };

    class GovernmentGrantConstructor: public ActionConstructor{
        const std::string movetype = "GOVERNMENTGRANT";
        std::array<std::array<Actions::GovernmentGrant*,49>,48> actionMap;
    public:
        GovernmentGrantConstructor();
        ~GovernmentGrantConstructor();

        // Get a representation
        std::string get_movetype();

        // how many legal actions there are
        int n_actions(Board::Board& game_board);
        Actions::Action* random_action(Board::Board& game_board); // return a random action of thie movetype with legal arguments
        std::vector<Actions::Action*> all_actions(Board::Board& game_board); //  a vector of all legal actions for this movetype (E.g. instantiate a list of Action objects with all possible attributes)
        bool legal(Board::Board& game_board); // whether there is any legal use of this movetype
    };

    class QuietNightConstructor: public ActionConstructor{
        const std::string movetype = "QUIETNIGHT";
        Actions::QuietNight* quietNightAction;
    public:
        QuietNightConstructor();
        ~QuietNightConstructor();

        // Get a representation
        std::string get_movetype();

        // how many legal actions there are
        int n_actions(Board::Board& game_board);
        Actions::Action* random_action(Board::Board& game_board); // return a random action of thie movetype with legal arguments
        std::vector<Actions::Action*> all_actions(Board::Board& game_board); //  a vector of all legal actions for this movetype (E.g. instantiate a list of Action objects with all possible attributes)
        bool legal(Board::Board& game_board); // whether there is any legal use of this movetype
    };

    // ===== DO NOTHING CONSTRUCTOR =====
    class DoNothingConstructor: public ActionConstructor{
        const std::string movetype = "DONOTHING";
        
        // Pointer to an existing doNothing Action
        Actions::Action* doNothingAction;
    public:
        DoNothingConstructor();
        ~DoNothingConstructor();

        // Get a representation
        std::string get_movetype();

        // how many legal actions there are
        int n_actions(Board::Board& game_board);
        Actions::Action* random_action(Board::Board& game_board); // return a random action of thie movetype with legal arguments
        std::vector<Actions::Action*> all_actions(Board::Board& game_board); //  a vector of all legal actions for this movetype (E.g. instantiate a list of Action objects with all possible attributes)
        bool legal(Board::Board& game_board); // whether there is any legal use of this movetype
    };

    // ===== FORCED DISCARD CONSTRUCTOR =====

    class ForcedDiscardConstructor: public Actions::ActionConstructor{
        const std::string movetype = "FORCEDDISCARD";

        std::vector<Actions::ForcedDiscardAction*> discardActions;

        // Also keep constructors for the event card actions, in case those are used in lieu of discard
        Actions::QuietNightConstructor* quietNightConstructor;
        Actions::GovernmentGrantConstructor* governmentGrantConstructor;
        Actions::AirliftConstructor* airliftConstructor;
    public:
        ForcedDiscardConstructor();
        ~ForcedDiscardConstructor();

        // Get a representation
        std::string get_movetype();

        // how many legal actions there are
        int n_actions(Board::Board& game_board);
        Actions::Action* random_action(Board::Board& game_board); // return a random action of thie movetype with legal arguments
        std::vector<Actions::Action*> all_actions(Board::Board& game_board); //  a vector of all legal actions for this movetype (E.g. instantiate a list of Action objects with all possible attributes)
        bool legal(Board::Board& game_board);
    };
}

#endif