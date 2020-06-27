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
    reset_board(new_board,verbose);
    return new_board;
}

void Scenarios::VanillaGameScenario::reset_board(Board::Board* game_board, bool verbose){
    game_board -> clear();
    game_board -> setup(verbose);
}

// Forced Discard Scenario
Scenarios::ForcedDiscardScenario::ForcedDiscardScenario():
    Scenario("Forced Discard Scenario","A Scenario where the initial player starts with too many (identical) event cards. Used to test ForcedDiscard action."){};

Board::Board* Scenarios::ForcedDiscardScenario::make_board(std::vector<int> roles,int _difficulty,bool verbose){
    // A scenario designed to test whether the ForcedDiscardConstructor can return both event card and citycard discard actions
    // Quarantine Specialist, Scientist, Researcher @ difficulty 4
    Board::Board* new_board = new Board::Board({0,2,3},4);
    // Use existing setup utility just to seed some hands
    reset_board(new_board,verbose);
    return new_board;
}

void Scenarios::ForcedDiscardScenario::reset_board(Board::Board* game_board,bool verbose){
    game_board -> clear();

    game_board -> setup(verbose);

    // Insert a lot of airlift cards in the hand of the first player
    // Compiler LOVES to skip this if I loop it
    game_board -> active_player().UpdateHand(50);
    game_board -> active_player().UpdateHand(50);
    game_board -> active_player().UpdateHand(50);
    game_board -> active_player().UpdateHand(50);
    game_board -> active_player().UpdateHand(50);
    game_board -> active_player().UpdateHand(50);
    game_board -> active_player().UpdateHand(50);

    for(int& card: game_board -> active_player().event_cards){
        DEBUG_MSG(std::endl << "[Scenarios::ForcedDiscardScenario()] "<< game_board -> active_player().role.name << " has event card: " << Decks::CARD_NAME(card));
    }
}

// "BusyBoard" scenario - research stations for moving around & cards for building new ones
Scenarios::BusyBoardScenario::BusyBoardScenario():
    Scenario("Busy Board Scenario","Have players start in a cluster of research stations with cards of their current and surrounding cities. For testing many movement options and trading."){};

Board::Board* Scenarios::BusyBoardScenario::make_board(std::vector<int> roles,int _difficulty,bool verbose){
    // A scenario designed to have an agent try lots of non-movement actions
    Board::Board* new_board = new Board::Board(roles,_difficulty);

    reset_board(new_board,verbose);
    return new_board;
}

void Scenarios::BusyBoardScenario::reset_board(Board::Board* game_board,bool verbose){
    game_board -> clear();
    // Put 6 research stations more-or-less next to eachother on the board somewhere;
    int random_city = rand() % Map::CITIES.size();

    // This doesn't make sense, but somehow arranging it this way actually forces the compiler to stop ignoring these commands
    game_board -> AddStation(random_city);
    game_board -> AddStation((random_city+1) % Map::CITIES.size());
    game_board -> AddStation((random_city+2) % Map::CITIES.size());
    game_board -> AddStation((random_city+3) % Map::CITIES.size());
    game_board -> AddStation((random_city+4) % Map::CITIES.size());
    game_board -> AddStation((random_city+5) % Map::CITIES.size());

    game_board -> reset_disease_count();
    game_board -> setup_player_deck();

    if(verbose){
        DEBUG_MSG(std::endl << "[Scenarios::BusyBoardTest()] This board has research stations at: ");
        for(int st: game_board -> get_stations()){
            DEBUG_MSG(Map::CITY_NAME(st) << ", ");
        }
        DEBUG_MSG(std::endl);
    }
    // Put each player in one of those cities with a station
    for(int p; p<game_board -> get_players().size();p++){
        game_board -> get_players()[p].set_position(Map::CITIES[(random_city+p )% Map::CITIES.size()]);

        // Give them a card of the city they're in
        game_board -> get_players()[p].UpdateHand((random_city+p )% Map::CITIES.size());
        // And a card for each neighboring city
        for(int neighbor: Map::CITIES[(random_city+p) % Map::CITIES.size()].neighbors){
            // and give them a card for each neighboring city - hopefully they try to build?
            game_board -> get_players()[p].UpdateHand(neighbor);
        }
        if(verbose){
            DEBUG_MSG("[Scenarios::BusyBoardTest()] " << game_board -> get_players()[p].role.name << " has cards: " );
            for(int& card: game_board ->get_players()[p].hand){
                DEBUG_MSG("[Scenarios::BusyBoardTest()] ... " << Decks::CARD_NAME(card) << std::endl);
            }
        }
    }

    game_board -> is_setup();
}

// "Can Win" scenario - put the players in a near-win scenario and see if they can win
Scenarios::CanWinScenario::CanWinScenario():
    Scenario("Can Win Scenario","A scenario where four players each start with cards required to cure a unique disease, in a board where all but one city has a research station. Initialize some disease on and around atlanta too to test TREAT behavior after cure. Designed to test win mechanic."){}

Board::Board* Scenarios::CanWinScenario::make_board(std::vector<int> roles,int _difficulty,bool verbose){
    // A scenario designed to have an agent try lots of non-movement actions
    Board::Board* new_board = new Board::Board({0,1,2,3},4);

    reset_board(new_board,verbose);
    return new_board;
}

void Scenarios::CanWinScenario::reset_board(Board::Board* game_board,bool verbose){

    game_board -> clear();

    game_board -> is_setup();
    game_board -> reset_disease_count();
    game_board -> setup_player_deck();

    // Do same ridiculous thing as BusyBoardTest to instantiate a lot of research stations
    // Infect atlanta and neighbors of atlanta with >1 cubes to test TREAT effect updating with cure
    game_board -> AddStation(0);
    game_board -> AddStation(1);
    std::array<int,2> junk = game_board -> infect(1,0,2);
    junk = game_board ->infect(1,1,2);
    junk = game_board ->infect(1,2,2);
    junk = game_board ->infect(1,3,2);
    game_board -> AddStation(2);
    game_board -> AddStation(3);
    junk = game_board -> infect(3,0,2);
    junk = game_board -> infect(3,1,2);
    junk = game_board -> infect(3,2,2);
    junk = game_board -> infect(3,3,2);
    game_board -> AddStation(4);
    junk = game_board -> infect(4,0,2);
    junk = game_board -> infect(4,1,2);
    junk = game_board -> infect(4,2,2);
    junk = game_board -> infect(4,3,2);
    game_board -> AddStation(5);
    game_board -> AddStation(6);
    game_board -> AddStation(7);
    game_board -> AddStation(8);
    game_board -> AddStation(9);
    game_board -> AddStation(10);
    game_board -> AddStation(11);
    game_board -> AddStation(12);
    game_board -> AddStation(13);
    game_board -> AddStation(14);
    junk = game_board -> infect(14,0,2);
    junk = game_board -> infect(14,1,2);
    junk = game_board -> infect(14,2,2);
    junk = game_board -> infect(14,3,2);
    game_board -> AddStation(15);
    game_board -> AddStation(16);
    game_board -> AddStation(17);
    game_board -> AddStation(18);
    game_board -> AddStation(19);
    game_board -> AddStation(20);
    game_board -> AddStation(21);
    game_board -> AddStation(22);
    game_board -> AddStation(23);
    game_board -> AddStation(24);
    game_board -> AddStation(25);
    game_board -> AddStation(26);
    game_board -> AddStation(27);
    game_board -> AddStation(28);
    game_board -> AddStation(29);
    game_board -> AddStation(30);
    game_board -> AddStation(31);
    game_board -> AddStation(32);
    game_board -> AddStation(33);
    game_board -> AddStation(34);
    game_board -> AddStation(35);
    game_board -> AddStation(36);
    game_board -> AddStation(37);
    game_board -> AddStation(38);
    game_board -> AddStation(39);
    game_board -> AddStation(40);
    game_board -> AddStation(41);
    game_board -> AddStation(42);
    game_board -> AddStation(43);
    game_board -> AddStation(44);
    game_board -> AddStation(45);
    game_board -> AddStation(46);
    // new_board.AddStation(Map::CITIES[47]); // Don't put one at at least one city, or else the algorithm for GovernmentGrantConstructor.random_action() never terminates!

    // Give each player 7 cards of a distinct color
    // blue card player
    game_board -> get_players()[Map::BLUE].UpdateHand(7);
    game_board -> get_players()[Map::BLUE].UpdateHand(8);
    game_board -> get_players()[Map::BLUE].UpdateHand(6);
    game_board -> get_players()[Map::BLUE].UpdateHand(9);
    game_board -> get_players()[Map::BLUE].UpdateHand(10);
    game_board -> get_players()[Map::BLUE].UpdateHand(11);
    game_board -> get_players()[Map::BLUE].UpdateHand(12);

    // yellow card player
    game_board -> get_players()[Map::YELLOW].UpdateHand(7+12);
    game_board -> get_players()[Map::YELLOW].UpdateHand(8+12);
    game_board -> get_players()[Map::YELLOW].UpdateHand(9+12);
    game_board -> get_players()[Map::YELLOW].UpdateHand(10+12);
    game_board -> get_players()[Map::YELLOW].UpdateHand(11+12);
    game_board -> get_players()[Map::YELLOW].UpdateHand(6+12);
    game_board -> get_players()[Map::YELLOW].UpdateHand(12+12);

    // black card player
    game_board -> get_players()[Map::BLACK].UpdateHand(6+24);
    game_board -> get_players()[Map::BLACK].UpdateHand(7+24);
    game_board -> get_players()[Map::BLACK].UpdateHand(8+24);
    game_board -> get_players()[Map::BLACK].UpdateHand(9+24);
    game_board -> get_players()[Map::BLACK].UpdateHand(10+24);
    game_board -> get_players()[Map::BLACK].UpdateHand(11+24);
    game_board -> get_players()[Map::BLACK].UpdateHand(12+24);

    // red card player
    game_board -> get_players()[Map::RED].UpdateHand(6+36);
    game_board -> get_players()[Map::RED].UpdateHand(7+36);
    game_board -> get_players()[Map::RED].UpdateHand(8+36);
    game_board -> get_players()[Map::RED].UpdateHand(9+36);
    game_board -> get_players()[Map::RED].UpdateHand(10+36);
    game_board -> get_players()[Map::RED].UpdateHand(11+36);
    game_board -> get_players()[Map::RED].UpdateHand(12+36);
}
