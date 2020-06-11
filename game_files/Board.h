#ifndef BOARD_H
#define BOARD_H

#include <vector>
#include <string>
#include <array>

#include "Map.h"
#include "Decks.h"
#include "Players.h"

namespace Board
{
    // store infection rate vector as global constant
    inline const std::vector<int> INFECTION_COUNTER = {2,2,2,3,3,4,4};

    class Board{
    private:
        // only tracks whether or not setup() has already been called.
        bool IS_SETUP = false;

        // the players
        std::vector<Players::Player> players = {};

        // where the research stations are
        std::vector<Map::City*> research_stations = {};

        // player deck
        Decks::PlayerDeck player_deck;
        
        // infect card deck
        Decks::InfectDeck infect_deck;

        int outbreak_count = 0; // increments on city outbreaks
        int epidemics_drawn = 0; // increments on drawing epidemics. Used to get infection rate from INFECTION_COUNTER

        // vector of 4 vectors : one to count each disease of size 48 (# cities)
        // have to initialize to 0 on setup
        std::array<std::array<int,48>,4> disease_count; 

        std::array<int,4> color_count = {0,0,0,0};
        // vector of 4 bools: whether or not each disease is cured
        std::vector<bool> cured= {false,false,false,false};
        // vector of 4 bools: whether or not each disease is eradicated
        std::vector<bool> eradicated = {false,false,false,false};

        // Tracks which players turn it is
        int turn = 0; 

        // This is perhaps a silly way to solve the tracking of already-outbroken cities during outbreak()
        std::vector<int> already_outbroken_cities = {};

        // Actions used on this turn so far. Also used to track game stage
        // {0,1,2,3} are player turns, incremented with each action.
        // {4} is player card draw stage, incremented after drawing second player card
        // {5} is infect stage is incremented 
        // End of infect stage should result in reset to 0
        int turn_actions = 0;

        // Tracked by stochastic actions. How many player cards have been drawn during draw phase.
        int player_cards_drawn = 0; 
        // Tracked by stochastic actions. How many infect cards have been drawn during this phase;
        int infect_cards_drawn = 0;

        bool quiet_night = false;

        bool lost = false;
        std::string why_lost;

        bool won = false;

        // flag that can be referenced with broken() to force failure on known badness in logic functions
        bool BROKEN = false;
        std::vector<std::string> why_it_broke; // can be used to update reasons BROKEN=true was set
    public:
        // Should only be called once instantiate the board.
        // Can input the player roles, hardcoded in Players.cpp switch() correspondence
        // Can input difficulty to describe how many epidemic cards to include in player deck.
        Board(std::vector<int> roles,int _difficulty);
        ~Board(){};

        // custom copy constructor
        Board(const Board& other){
            // ===== explicitly copy everything - I'm not convinced it was doing any of this
            IS_SETUP = other.IS_SETUP;

            outbreak_count = other.outbreak_count;
            epidemics_drawn = other.epidemics_drawn;

            player_cards_drawn = other.player_cards_drawn;
            infect_cards_drawn = other.infect_cards_drawn;

            turn_actions = other.turn_actions;
            turn = other.turn;

            // These copies really shouldn't be necessary in most implementations but w/e
            lost = other.lost;
            won = other.won;
            BROKEN = other.BROKEN;

            // ===== End of copy of basic attributes =====
            // I _think_ this is a silly but at least deep copy method
            players.clear();
            for(int p=0;p<other.players.size();p++){
                // explicit copy
                Players::Player pl = other.players[p];
                players.push_back(pl);
            }
            
            research_stations.clear();
            for(Map::City* rst : other.research_stations){
                research_stations.push_back(rst);
            }

            cured = other.cured;
            eradicated = other.eradicated;

            disease_count = other.disease_count;

            // Both of these require an overloaded = operator
            player_deck = other.player_deck;
            infect_deck = other.infect_deck;
        }

        std::string repr(); // A string representation for logging (unimplemented rn)

        // Setup the board for play
        void setup(bool verbose=false);
        // Clear the board to reset to just-constructed status
        void clear();

        void setup_player_deck();
        void is_setup();

        // difficulty. Only used for sanity checks after initialization
        int difficulty = 0;

        // Logic to add cubes to city
        // incorporates outbreak etc.
        // returns vector of <# cities outbroken, # prevented> for logging
        // obviously isn't useful for tracking how many cubes the specialist blocked, only prevented outbreaks
        
        // This is the function to call to actually infect cities. Requires resetting memory o
        std::array<int,2> infect(int city_idx,int color, int add);

        // This does the actual infecting of each city
        std::array<int,2> infect_city(int city_idx,int color, int add);

        // Logic to outbreak a city
        std::array<int,2> outbreak(Map::City& city,int col);
        std::array<int,2> outbreak(int city_idx, int col);

        // is_terminal and valuation
        bool is_terminal();
        int win_lose();

        // Check all gamestate variables and set win/lose/break as appropriate, if applicable
        void updatestatus();
        void update_medic_position(); // Check whether there's a medic and they're on a city with a cured disease, and remove any disease of that color if so
        void update_eradicated_status(); // Update eradicated status according to cure status and disease_count

        // functional additions (lots of them)
        Players::Player& active_player(); // reference to active player
        std::array<std::array<int,48>,4>& get_disease_count(); // reference to disease count to increment
        std::array<int,4>& get_color_count();
        int disease_sum(int col); // sum of disease cubes of a color
        void reset_disease_count();
        void reset_outbreak_memory();

        std::vector<Map::City*>& get_stations(); // reference to research stations
        void AddStation(Map::City& new_station);
        void RemoveStation(int station_list_idx);
                
        bool is_eradicated(int col); // get eradicated status of a disease
        void Eradicate(int col); // set eradicated status of a disease to true

        bool is_cured(int col); // get cured status of given disease
        void Cure(int color); // Tell the board to cure a disease (shouldn't be necessary but optimizer doesn't like updating status occasionally when told to do so)

        std::vector<Players::Player>& get_players();
        bool& quiet_night_status();
        bool& has_won(); // reference to win status. Only modified in Action Actions::Cure
        bool& has_lost(); // reference to loss status. Only modified during stochasticity
        int& get_turn_action();

        // Player deck entrypoints
        int draw_playerdeck_inplace();
        int& get_player_cards_drawn(); // Entry point for access to number of player cards drawn during player draw phase
        void updatePlayerDeck(int card); // update player deck to reflect a card having been drawn and used
        int remaining_player_cards(); // Entry point for asking how many player cards are left in the deck
        bool epidemic_possible(); // Is it possible to draw an epidemic on the next player card?
        int get_epidemic_count();

        // Infect deck entrypoints
        int draw_infectdeck_bottom_inplace();// return a card that might be drawn without removing it from deck
        int draw_infectdeck_inplace();
        void readd_infect_discard(); // Logic to re-add discard to top of infect deck
        void updateInfectDeck(int card,bool bottom=false); // update infect deck to reflect a card having been drawn and used
        int& get_infect_cards_drawn();
        int n_infect_cards(bool top=true); // number of infect cards in the group on top (true) or bottom (false) of infect deck

        int& get_outbreak_count();
        int& get_difficulty();
        int get_infection_rate(); // return the current infection rate according to kept index
        int& get_turn(); // return reference to whose turn it is

        bool disease_count_safe();
        bool player_deck_nonempty();
        bool outbreak_count_safe();
        
        // use it to force breaking in bad logic cases.
        bool& broken();
        // reference to why_it_broke vector - insert new reasons as more things break!
        std::vector<std::string>& broken_reasons();
        std::string& get_lost_reason();

    };
}

#endif