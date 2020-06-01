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
        std::vector<Map::City> research_stations={};

        // player deck
        Decks::PlayerDeck player_deck;
        
        // infect card deck
        Decks::InfectDeck infect_deck;

        int outbreak_count = 0; // increments on city outbreaks
        int epidemics_drawn = 0; // increments on drawing epidemics. Used to get infection rate from INFECTION_COUNTER

        // vector of 4 vectors : one to count each disease of size 48 (# cities)
        // have to initialize to 0 on setup
        std::array<std::array<int,48>,4> disease_count; 

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
        std::string repr(); // A string representation for logging

        // Setup the board for play
        void setup(bool verbose=false);
        void setup_player_deck();
        void is_setup();

        // Logic to draw from the player deck
        Decks::PlayerCard draw_playerdeck();
        // Logic to draw from infect deck
        Decks::InfectCard draw_infectdeck();
        // Logic to draw from bottom of infect deck
        Decks::InfectCard draw_infectdeck_bottom();
        // Logic to re-add discard to top of infect deck
        void readd_infect_discard();

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
        std::array<int,2> outbreak(Map::City city,int col);
        std::array<int,2> outbreak(int city_idx, int col);

        // is_terminal and valuation
        bool is_terminal();
        int win_lose();

        // Check all gamestate variables and set win/lose/break as appropriate, if applicable
        void updatestatus();
        void update_medic_position(); // Check whether there's a medic and they're on a city with a cured disease
        void update_eradicated_status(); // Update eradicated status according to cure status and disease_count

        // functional additions (lots of them)
        Players::Player& active_player(); // reference to active player
        std::array<std::array<int,48>,4>& get_disease_count(); // reference to disease count to increment
        int disease_sum(int col); // sum of disease cubes of a color
        void reset_disease_count();
        void reset_outbreak_memory();

        std::vector<Map::City> get_stations(); // reference to research stations
        void AddStation(Map::City new_station);
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
        int& get_player_cards_drawn(); // Entry point for access to number of player cards drawn during player draw phase
        int remaining_player_cards(); // Entry point for asking how many player cards are left in the deck
        int& get_infect_cards_drawn();
        int& get_outbreak_count();
        int get_epidemic_count();
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