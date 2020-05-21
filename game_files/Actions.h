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
        Action(Board::Board& _active_board);
        // A pointer to the board to be modified in place by execute()
        Board::Board *active_board;

        // E.g. "MOVE"
        std::string movetype;

        // Act on the referenced active_board according to child class logic
        virtual void execute()=0;
        
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

            <Build> a research station

            <Treat> disease on a city
            <Cure> disease at a city

            <Give> a card to another player
            <Take> a card from another player
        
        Event Card actions:
            <Airlift>: move any player to any location
            <GovernmentGrant>: build a research station anywhere
            <QuietNight>: skip the next infect step

        NULL action:
            <DoNothing> You're allowed to do nothing!
    */

    // ===== Move ===== 
    class Move: public Action{
        // to incorporate dispatcher would either need a new action, or include player, from, and to here.
        Map::City to;
    public:
        Move(Board::Board& _active_board, Map::City _to);
        void execute(); // to modify active_board in-place
        std::string repr(); // to yield a string representation for logging
    };

    // ===== DirectFlight ===== 
    class DirectFlight: public Action{
        Decks::PlayerCard citycard; // card to discard
    public:
        DirectFlight(Board::Board& _active_board,int hand_idx); // instantiate with index in hand (useful for randomization)
        DirectFlight(Board::Board& _active_board,Decks::CityCard _citycard);
        void execute(); // to modify active_board
        std::string repr(); // to yield a string representation for logging
    };

    // ===== CharterFlight =====
    class CharterFlight: public Action{
        Map::City target_city;
    public:
        CharterFlight(Board::Board& _active_board, int _target_city);
        CharterFlight(Board::Board& _active_board, Map::City _target_city);
        void execute(); // to modify active_board
        std::string repr(); // to yield a string representation for logging
    };

    // ===== ShuttleFlight =====
    class ShuttleFlight: public Action{
        Map::City target_station;
    public:
        ShuttleFlight(Board::Board& _active_board, int _target_station);
        ShuttleFlight(Board::Board& _active_board, Map::City _target_station);
        void execute(); // to modify active_board
        std::string repr(); // to yield a string representation for logging
    };

    // ===== OperationsExpertFlight =====
    // (Like shuttle flight, but to ANY city)
    class OperationsExpertFlight: public Action{
        Map::City target_city;
        Decks::CityCard discard_card;
    public:
        OperationsExpertFlight(Board::Board& _active_board, int _target_city,Decks::CityCard _discard);
        OperationsExpertFlight(Board::Board& _active_board, Map::City _target_city,Decks::CityCard _discard);
        OperationsExpertFlight(Board::Board& _active_board, int _target_city,int _discard_city_idx);
        void execute(); // to modify active_board
        std::string repr(); // to yield a string representation for logging
    };

    // ===== Build =====
    class Build: public Action{
        // Will have to account for presence of existing stations.
        // This is the index of the station in the list of stations to remove
        int remove_station;
    public:
        Build(Board::Board& _active_board,int _remove_station=-1);
        void execute(); // to modify active_board
        std::string repr(); // to yield a string representation for logging
    }; 

    // ===== Treat ===== 
    class Treat: public Action{
        int color; // color to target
    public:
        Treat(Board::Board& _active_board, int _color);
        void execute(); // to modify active_board
        std::string repr(); // to yield a string representation for logging
    };

    // ===== Cure ===== 
    class Cure: public Action{
        // when available b/c it's legal, there can only be one curable color (at least 4 cards of hand size 7 are needed)
    public:
        Cure(Board::Board& _active_board);
        void execute(); // to modify active_board
        std::string repr(); // to yield a string representation for logging
    };

    // ===== Give ===== 
    class Give: public Action{
        Players::Player& other_player;
        Decks::CityCard card_to_give;
    public:
        Give(Board::Board& _active_board, Players::Player& _other_player, Decks::CityCard _card_to_give);
        Give(Board::Board& _active_board, Players::Player& _other_player, int _card_to_give_cityidx);
        void execute(); // to modify active_board
        std::string repr(); // to yield a string representation for logging
    };

    // ===== Take ===== 
    class Take: public Action{
        Players::Player& other_player;
        Decks::CityCard card_to_take;
    public:
        Take(Board::Board& _active_board, Players::Player& _other_player, Decks::CityCard _card_to_take);
        Take(Board::Board& _active_board, Players::Player& _other_player, int _card_to_take_city_idx);
        void execute(); // to modify active_board
        std::string repr(); // to yield a string representation for logging
    };

    // ===== EVENT CARD ACTIONS =====
    // ===== Airlift ===== 
    class Airlift: public Action{
        Players::Player& target_player; // Player to move
        Players::Player& using_player;
        int target_city; // City to move active 
    public:
        Airlift(Board::Board& _active_board, Players::Player& _using_player,Players::Player& _target_player,int _target_city);
        void execute(); // to modify active_board
        std::string repr(); // to yield a string representation for logging
        
    };

    // ===== Government Grant ===== 
    class GovernmentGrant: public Action{
        int target_city;
        Players::Player& using_player;
        int remove_station;
    public:
        GovernmentGrant(Board::Board& _active_board, Players::Player& _using_player,int _target_city,int _remove_station=-1);
        void execute(); // to modify active_board
        std::string repr(); // to yield a string representation for logging
    };

    // ===== Quiet Night ===== 
    class QuietNight: public Action{
        Players::Player& using_player;
    public:
        QuietNight(Board::Board& _active_board,Players::Player& _using_player);
        void execute(); // to modify active_board
        std::string repr(); // to yield a string representation for logging
    };

    // ===== Forced Discard Action 
    class ForcedDiscardAction: public Actions::Action{
    Decks::PlayerCard discard_card;
    Players::Player& player_to_discard;
    public:
        ForcedDiscardAction(Board::Board& _active_board,Players::Player& player_to_discard,Decks::PlayerCard _discard_card);
        void execute();
        std::string repr();
        bool legal();
    };

    // ===== NULL ACTION ===== 
    // All other actions are coded so that this is UNIQUE;
    // ===== Do Nothing =====
    class DoNothing: public Action{
    public:
        DoNothing(Board::Board& _active_board);
        void execute(); // to modify active_board
        std::string repr(); // to yield a string representation for logging
    };

    //This is an object that can instantiate any of a class of actions, using legal arguments.
    class ActionConstructor{
    public:
        ActionConstructor(Board::Board& _active_board);
        Board::Board *active_board;
        
        // Base Constructor methods are all pure virtual - REQUIRES child to be defined. Good b/c this class should never be called.
        
        // Get a representation
        virtual std::string get_movetype()=0;
        // how many legal actions there are
        virtual int n_actions()=0; 
        virtual Actions::Action* random_action()=0; // return a random action of thie movetype with legal arguments
        virtual std::vector<Actions::Action*> all_actions()=0; //  a vector of all legal actions for this movetype (E.g. instantiate a list of Action objects with all possible attributes)
        virtual bool legal()=0;
    };

    // ===== PLAYER ACTION CONSTRUCTORS =====
    class MoveConstructor: public ActionConstructor{
        const std::string movetype = "MOVE"; // technically "Drive/Ferry"
        std::vector<int> legal_moves; // adjacent city indices
    public:
        MoveConstructor(Board::Board& active_board);

        // Get a representation
        virtual std::string get_movetype();

        // how many legal actions there are
        int n_actions(); 
        Actions::Action* random_action(); // return a random action of thie movetype with legal arguments
        std::vector<Actions::Action*> all_actions(); //  a vector of all legal actions for this movetype (E.g. instantiate a list of Action objects with all possible attributes)
        bool legal(); // whether there is any legal use of this movetype
    };

    class DirectFlightConstructor: public ActionConstructor{
        const std::string movetype = "DIRECTFLIGHT";
    public:
        DirectFlightConstructor(Board::Board& active_board);

        // Get a representation
        virtual std::string get_movetype();

        // how many legal actions there are
        int n_actions();
        Actions::Action* random_action(); // return a random action of thie movetype with legal arguments
        std::vector<Actions::Action*> all_actions(); //  a vector of all legal actions for this movetype (E.g. instantiate a list of Action objects with all possible attributes)
        bool legal(); // whether there is any legal use of this movetype
    };

    class CharterFlightConstructor: public ActionConstructor{
        const std::string movetype = "CHARTERFLIGHT";
        // If available, always 47 actions: one for each city to fly to that's not the player's current one.
    public:
        CharterFlightConstructor(Board::Board& active_board);

        // Get a representation
        virtual std::string get_movetype();

        // how many legal actions there are
        int n_actions();
        Actions::Action* random_action(); // return a random action of thie movetype with legal arguments
        std::vector<Actions::Action*> all_actions(); //  a vector of all legal actions for this movetype (E.g. instantiate a list of Action objects with all possible attributes)
        bool legal(); // whether there is any legal use of this movetype
    };

    class ShuttleFlightConstructor: public ActionConstructor{
        const std::string movetype = "SHUTTLEFLIGHT";
        // If it's legal we can move to any of the other research stations
    public:
        ShuttleFlightConstructor(Board::Board& active_board);

        // Get a representation
        virtual std::string get_movetype();

        // how many legal actions there are
        int n_actions();
        Actions::Action* random_action(); // return a random action of thie movetype with legal arguments
        std::vector<Actions::Action*> all_actions(); //  a vector of all legal actions for this movetype (E.g. instantiate a list of Action objects with all possible attributes)
        bool legal(); // whether there is any legal use of this movetype
    };

    class OperationsExpertFlightConstructor: public ActionConstructor{
        const std::string movetype = "OPERATIONSEXPERTFLIGHT";
        // If it's legal we can move to any of the other research stations
    public:
        OperationsExpertFlightConstructor(Board::Board& active_board);

        // Get a representation
        virtual std::string get_movetype();

        // how many legal actions there are
        int n_actions();
        Actions::Action* random_action(); // return a random action of thie movetype with legal arguments
        std::vector<Actions::Action*> all_actions(); //  a vector of all legal actions for this movetype (E.g. instantiate a list of Action objects with all possible attributes)
        bool legal(); // whether there is any legal use of this movetype
    };

    class BuildConstructor: public ActionConstructor{
        const std::string movetype = "BUILD";
        // If it's legal we can build a research station at the position of the current player.
    public:
        BuildConstructor(Board::Board& active_board);

        // Get a representation
        virtual std::string get_movetype();

        // how many legal actions there are
        int n_actions();
        Actions::Action* random_action(); // return a random action of thie movetype with legal arguments
        std::vector<Actions::Action*> all_actions(); //  a vector of all legal actions for this movetype (E.g. instantiate a list of Action objects with all possible attributes)
        bool legal(); // whether there is any legal use of this movetype
    };

    class TreatConstructor: public ActionConstructor{
        const std::string movetype = "TREAT";
    public:
        TreatConstructor(Board::Board& active_board);

        // Get a representation
        virtual std::string get_movetype();

        // how many legal actions there are
        int n_actions();
        Actions::Action* random_action(); // return a random action of thie movetype with legal arguments
        std::vector<Actions::Action*> all_actions(); //  a vector of all legal actions for this movetype (E.g. instantiate a list of Action objects with all possible attributes)
        bool legal(); // whether there is any legal use of this movetype
    };

    class CureConstructor: public ActionConstructor{
        const std::string movetype = "CURE";
    public:
        CureConstructor(Board::Board& active_board);

        // Get a representation
        virtual std::string get_movetype();

        // how many legal actions there are
        int n_actions();
        Actions::Action* random_action(); // return a random action of thie movetype with legal arguments
        std::vector<Actions::Action*> all_actions(); //  a vector of all legal actions for this movetype (E.g. instantiate a list of Action objects with all possible attributes)
        bool legal(); // whether there is any legal use of this movetype
    };

    class GiveConstructor: public ActionConstructor{
        const std::string movetype = "GIVE";
    public:
        GiveConstructor(Board::Board& active_board);

        // Get a representation
        virtual std::string get_movetype();

        // how many legal actions there are
        int n_actions();
        Actions::Action* random_action(); // return a random action of thie movetype with legal arguments
        std::vector<Actions::Action*> all_actions(); //  a vector of all legal actions for this movetype (E.g. instantiate a list of Action objects with all possible attributes)
        bool legal(); // whether there is any legal use of this movetype
    };

    class TakeConstructor: public ActionConstructor{
        const std::string movetype = "TAKE";
    public:
        TakeConstructor(Board::Board& active_board);

        // Get a representation
        virtual std::string get_movetype();

        // how many legal actions there are
        int n_actions();
        Actions::Action* random_action(); // return a random action of thie movetype with legal arguments
        std::vector<Actions::Action*> all_actions(); //  a vector of all legal actions for this movetype (E.g. instantiate a list of Action objects with all possible attributes)
        bool legal(); // whether there is any legal use of this movetype
    };

    // ===== EVENT CARD ACTION CONSTRUCTORS =====

    // Parent to generate all available event card actions
    // (Deprecated - GameLogic looks at individual EventCard action constructors individually)
    // class EventCardActionConstructor: public ActionConstructor{
    // public:
    //     EventCardActionConstructor(Board::Board& _active_board);

    //     // how many legal actions there are
    //     virtual int n_actions();
    //     virtual Action* random_action(); // return a random action of thie movetype with legal arguments
    //     virtual std::vector<Actions::Action*> all_actions(); //  a vector of all legal actions for this movetype (E.g. instantiate a list of Action objects with all possible attributes)
    //     virtual bool legal();
    // };
    
    class AirliftConstructor: public ActionConstructor{
        const std::string movetype = "AIRLIFT";
    public:
        AirliftConstructor(Board::Board& active_board);

        // Get a representation
        virtual std::string get_movetype();

        // how many legal actions there are
        int n_actions();
        Actions::Action* random_action(); // return a random action of thie movetype with legal arguments
        std::vector<Actions::Action*> all_actions(); //  a vector of all legal actions for this movetype (E.g. instantiate a list of Action objects with all possible attributes)
        bool legal(); // whether there is any legal use of this movetype
    };

    class GovernmentGrantConstructor: public ActionConstructor{
        const std::string movetype = "GOVERNMENTGRANT";
    public:
        GovernmentGrantConstructor(Board::Board& active_board);

        // Get a representation
        virtual std::string get_movetype();

        // how many legal actions there are
        int n_actions();
        Actions::Action* random_action(); // return a random action of thie movetype with legal arguments
        std::vector<Actions::Action*> all_actions(); //  a vector of all legal actions for this movetype (E.g. instantiate a list of Action objects with all possible attributes)
        bool legal(); // whether there is any legal use of this movetype
    };

    class QuietNightConstructor: public ActionConstructor{
        const std::string movetype = "QUIETNIGHT";
    public:
        QuietNightConstructor(Board::Board& active_board);

        // Get a representation
        virtual std::string get_movetype();

        // how many legal actions there are
        int n_actions();
        Actions::Action* random_action(); // return a random action of thie movetype with legal arguments
        std::vector<Actions::Action*> all_actions(); //  a vector of all legal actions for this movetype (E.g. instantiate a list of Action objects with all possible attributes)
        bool legal(); // whether there is any legal use of this movetype
    };

    // ===== DO NOTHING CONSTRUCTOR =====
    class DoNothingConstructor: public ActionConstructor{
        const std::string movetype = "DONOTHING";
    public:
        DoNothingConstructor(Board::Board& active_board);

        // Get a representation
        virtual std::string get_movetype();

        // how many legal actions there are
        int n_actions();
        Actions::Action* random_action(); // return a random action of thie movetype with legal arguments
        std::vector<Actions::Action*> all_actions(); //  a vector of all legal actions for this movetype (E.g. instantiate a list of Action objects with all possible attributes)
        bool legal(); // whether there is any legal use of this movetype
    };

    // ===== FORCED DISCARD CONSTRUCTOR =====

    class ForcedDiscardConstructor: public Actions::ActionConstructor{
        const std::string movetype = "FORCEDDISCARD";
    public:
        ForcedDiscardConstructor(Board::Board& _active_board);

        // Get a representation
        virtual std::string get_movetype();

        // how many legal actions there are
        int n_actions();
        Actions::Action* random_action(); // return a random action of thie movetype with legal arguments
        std::vector<Actions::Action*> all_actions(); //  a vector of all legal actions for this movetype (E.g. instantiate a list of Action objects with all possible attributes)
        bool legal();
    };
}

#endif