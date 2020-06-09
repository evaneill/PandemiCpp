#include "Scenarios.h"

#include "../game_files/Board.h"

Scenarios::Scenario::Scenario(std::string _name,std::string desc){
    name = _name;
    description = desc;
}

// Vanilla game
Scenarios::VanillaGameScenario::VanillaGameScenario():
    Scenario("Vanilla Game","A Game board set up with given roles and difficulty according to official game rules")
{}

Board::Board* Scenarios::VanillaGameScenario::make_board(std::vector<int> roles,int _difficulty,bool verbose){
    Board::Board* new_board = new Board::Board(roles,_difficulty);
    new_board -> setup(verbose);
    return new_board;
}

// Forced Discard Scenario
Scenarios::ForcedDiscardScenario::ForcedDiscardScenario():
    Scenario("Forced Discard Scenario","A Scenario where the initial player starts with too many (identical) event cards. Used to test ForcedDiscard action."){};

Board::Board* Scenarios::ForcedDiscardScenario::make_board(std::vector<int> roles,int _difficulty,bool verbose){
    // A scenario designed to test whether the ForcedDiscardConstructor can return both event card and citycard discard actions
    // Quarantine Specialist, Scientist, Researcher @ difficulty 4
    Board::Board* new_board = new Board::Board({0,2,3},4);
    // Use existing setup utility just to seed some hands
    new_board -> setup(verbose);

    // Insert a lot of airlift cards in the hand of the first player
    // Compiler LOVES to skip this if I loop it
    new_board -> active_player().UpdateHand(Decks::PlayerCard(50));
    new_board -> active_player().UpdateHand(Decks::PlayerCard(50));
    new_board -> active_player().UpdateHand(Decks::PlayerCard(50));
    new_board -> active_player().UpdateHand(Decks::PlayerCard(50));
    new_board -> active_player().UpdateHand(Decks::PlayerCard(50));
    new_board -> active_player().UpdateHand(Decks::PlayerCard(50));
    new_board -> active_player().UpdateHand(Decks::PlayerCard(50));

    for(Decks::PlayerCard& card: new_board -> active_player().event_cards){
        DEBUG_MSG(std::endl << "[Scenarios::ForcedDiscardScenario()] "<< new_board -> active_player().role.name << " has event card: " << card.name);
    }

    return new_board;
}

// "BusyBoard" scenario - research stations for moving around & cards for building new ones
Scenarios::BusyBoardScenario::BusyBoardScenario():
    Scenario("Busy Board Scenario","Have players start in a cluster of research stations with cards of their current and surrounding cities. For testing many movement options and trading."){};

Board::Board* Scenarios::BusyBoardScenario::make_board(std::vector<int> roles,int _difficulty,bool verbose){
    // A scenario designed to have an agent try lots of non-movement actions
    Board::Board* new_board = new Board::Board(roles,_difficulty);

    // Put 6 research stations more-or-less next to eachother on the board somewhere;
    int random_city = rand() % Map::CITIES.size();

    // This doesn't make sense, but somehow arranging it this way actually forces the compiler to stop ignoring these commands
    new_board -> AddStation(Map::CITIES[random_city]);
    new_board -> AddStation(Map::CITIES[(random_city+1) % Map::CITIES.size()]);
    new_board -> AddStation(Map::CITIES[(random_city+2) % Map::CITIES.size()]);
    new_board -> AddStation(Map::CITIES[(random_city+3) % Map::CITIES.size()]);
    new_board -> AddStation(Map::CITIES[(random_city+4) % Map::CITIES.size()]);
    new_board -> AddStation(Map::CITIES[(random_city+5) % Map::CITIES.size()]);

    new_board -> reset_disease_count();
    new_board -> setup_player_deck();

    if(verbose){
        DEBUG_MSG(std::endl << "[Scenarios::BusyBoardTest()] This board has research stations at: ");
        for(Map::City& st: new_board -> get_stations()){
            DEBUG_MSG(st.name << ", ");
        }
        DEBUG_MSG(std::endl);
    }
    // Put each player in one of those cities with a station
    for(int p; p<new_board -> get_players().size();p++){
        new_board -> get_players()[p].set_position(Map::CITIES[(random_city+p )% Map::CITIES.size()]);

        // Give them a card of the city they're in
        new_board -> get_players()[p].UpdateHand(Decks::PlayerCard((random_city+p )% Map::CITIES.size()));
        // And a card for each neighboring city
        for(int neighbor: Map::CITIES[(random_city+p) % Map::CITIES.size()].neighbors){
            // and give them a card for each neighboring city - hopefully they try to build?
            new_board -> get_players()[p].UpdateHand(Decks::PlayerCard(neighbor));
        }
        if(verbose){
            DEBUG_MSG("[Scenarios::BusyBoardTest()] " << new_board -> get_players()[p].role.name << " has cards: " );
            for(Decks::PlayerCard& card: new_board ->get_players()[p].hand){
                DEBUG_MSG("[Scenarios::BusyBoardTest()] ... " << card.name << std::endl);
            }
        }
    }

    new_board -> is_setup();
    return new_board;
}

// "Can Win" scenario - put the players in a near-win scenario and see if they can win
Scenarios::CanWinScenario::CanWinScenario():
    Scenario("Can Win Scenario","A scenario where four players each start with cards required to cure a unique disease, in a board where all but one city has a research station. Initialize some disease on and around atlanta too to test TREAT behavior after cure. Designed to test win mechanic."){}

Board::Board* Scenarios::CanWinScenario::make_board(std::vector<int> roles,int _difficulty,bool verbose){
    // A scenario designed to have an agent try lots of non-movement actions
    Board::Board* new_board = new Board::Board({0,1,2,3},4);

    new_board -> is_setup();
    new_board -> reset_disease_count();
    new_board -> setup_player_deck();

    // Do same ridiculous thing as BusyBoardTest to instantiate a lot of research stations
    // Infect atlanta and neighbors of atlanta with >1 cubes to test TREAT effect updating with cure
    new_board -> AddStation(Map::CITIES[0]);
    new_board -> AddStation(Map::CITIES[1]);
    std::array<int,2> junk = new_board -> infect(1,0,2);
    junk = new_board ->infect(1,1,2);
    junk = new_board ->infect(1,2,2);
    junk = new_board ->infect(1,3,2);
    new_board -> AddStation(Map::CITIES[2]);
    new_board -> AddStation(Map::CITIES[3]);
    junk = new_board -> infect(3,0,2);
    junk = new_board -> infect(3,1,2);
    junk = new_board -> infect(3,2,2);
    junk = new_board -> infect(3,3,2);
    new_board -> AddStation(Map::CITIES[4]);
    junk = new_board -> infect(4,0,2);
    junk = new_board -> infect(4,1,2);
    junk = new_board -> infect(4,2,2);
    junk = new_board -> infect(4,3,2);
    new_board -> AddStation(Map::CITIES[5]);
    new_board -> AddStation(Map::CITIES[6]);
    new_board -> AddStation(Map::CITIES[7]);
    new_board -> AddStation(Map::CITIES[8]);
    new_board -> AddStation(Map::CITIES[9]);
    new_board -> AddStation(Map::CITIES[10]);
    new_board -> AddStation(Map::CITIES[11]);
    new_board -> AddStation(Map::CITIES[12]);
    new_board -> AddStation(Map::CITIES[13]);
    new_board -> AddStation(Map::CITIES[14]);
    junk = new_board -> infect(14,0,2);
    junk = new_board -> infect(14,1,2);
    junk = new_board -> infect(14,2,2);
    junk = new_board -> infect(14,3,2);
    new_board -> AddStation(Map::CITIES[15]);
    new_board -> AddStation(Map::CITIES[16]);
    new_board -> AddStation(Map::CITIES[17]);
    new_board -> AddStation(Map::CITIES[18]);
    new_board -> AddStation(Map::CITIES[19]);
    new_board -> AddStation(Map::CITIES[20]);
    new_board -> AddStation(Map::CITIES[21]);
    new_board -> AddStation(Map::CITIES[22]);
    new_board -> AddStation(Map::CITIES[23]);
    new_board -> AddStation(Map::CITIES[24]);
    new_board -> AddStation(Map::CITIES[25]);
    new_board -> AddStation(Map::CITIES[26]);
    new_board -> AddStation(Map::CITIES[27]);
    new_board -> AddStation(Map::CITIES[28]);
    new_board -> AddStation(Map::CITIES[29]);
    new_board -> AddStation(Map::CITIES[30]);
    new_board -> AddStation(Map::CITIES[31]);
    new_board -> AddStation(Map::CITIES[32]);
    new_board -> AddStation(Map::CITIES[33]);
    new_board -> AddStation(Map::CITIES[34]);
    new_board -> AddStation(Map::CITIES[35]);
    new_board -> AddStation(Map::CITIES[36]);
    new_board -> AddStation(Map::CITIES[37]);
    new_board -> AddStation(Map::CITIES[38]);
    new_board -> AddStation(Map::CITIES[39]);
    new_board -> AddStation(Map::CITIES[40]);
    new_board -> AddStation(Map::CITIES[41]);
    new_board -> AddStation(Map::CITIES[42]);
    new_board -> AddStation(Map::CITIES[43]);
    new_board -> AddStation(Map::CITIES[44]);
    new_board -> AddStation(Map::CITIES[45]);
    new_board -> AddStation(Map::CITIES[46]);
    // new_board.AddStation(Map::CITIES[47]); // Don't put one at at least one city, or else the algorithm for GovernmentGrantConstructor.random_action() never terminates!

    // Give each player 7 cards of a distinct color
    // blue card player
    new_board -> get_players()[Map::BLUE].UpdateHand(Decks::PlayerCard(7));
    new_board -> get_players()[Map::BLUE].UpdateHand(Decks::PlayerCard(8));
    new_board -> get_players()[Map::BLUE].UpdateHand(Decks::PlayerCard(6));
    new_board -> get_players()[Map::BLUE].UpdateHand(Decks::PlayerCard(9));
    new_board -> get_players()[Map::BLUE].UpdateHand(Decks::PlayerCard(10));
    new_board -> get_players()[Map::BLUE].UpdateHand(Decks::PlayerCard(11));
    new_board -> get_players()[Map::BLUE].UpdateHand(Decks::PlayerCard(12));

    // yellow card player
    new_board -> get_players()[Map::YELLOW].UpdateHand(Decks::PlayerCard(7+12));
    new_board -> get_players()[Map::YELLOW].UpdateHand(Decks::PlayerCard(8+12));
    new_board -> get_players()[Map::YELLOW].UpdateHand(Decks::PlayerCard(9+12));
    new_board -> get_players()[Map::YELLOW].UpdateHand(Decks::PlayerCard(10+12));
    new_board -> get_players()[Map::YELLOW].UpdateHand(Decks::PlayerCard(11+12));
    new_board -> get_players()[Map::YELLOW].UpdateHand(Decks::PlayerCard(6+12));
    new_board -> get_players()[Map::YELLOW].UpdateHand(Decks::PlayerCard(12+12));

    // black card player
    new_board -> get_players()[Map::BLACK].UpdateHand(Decks::PlayerCard(6+24));
    new_board -> get_players()[Map::BLACK].UpdateHand(Decks::PlayerCard(7+24));
    new_board -> get_players()[Map::BLACK].UpdateHand(Decks::PlayerCard(8+24));
    new_board -> get_players()[Map::BLACK].UpdateHand(Decks::PlayerCard(9+24));
    new_board -> get_players()[Map::BLACK].UpdateHand(Decks::PlayerCard(10+24));
    new_board -> get_players()[Map::BLACK].UpdateHand(Decks::PlayerCard(11+24));
    new_board -> get_players()[Map::BLACK].UpdateHand(Decks::PlayerCard(12+24));

    // red card player
    new_board -> get_players()[Map::RED].UpdateHand(Decks::PlayerCard(6+36));
    new_board -> get_players()[Map::RED].UpdateHand(Decks::PlayerCard(7+36));
    new_board -> get_players()[Map::RED].UpdateHand(Decks::PlayerCard(8+36));
    new_board -> get_players()[Map::RED].UpdateHand(Decks::PlayerCard(9+36));
    new_board -> get_players()[Map::RED].UpdateHand(Decks::PlayerCard(10+36));
    new_board -> get_players()[Map::RED].UpdateHand(Decks::PlayerCard(11+36));
    new_board -> get_players()[Map::RED].UpdateHand(Decks::PlayerCard(12+36));

    return new_board;
}
