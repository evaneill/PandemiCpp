#ifndef SCENARIOS_H
#define SCENARIOS_H

#include "../game_files/Board.h"

#include "../game_files/Decks.h"
#include "../game_files/Players.h"

#include "../game_files/Debug.h"

namespace Scenarios
{
    // This is the simplest example: Use the existing setup() method and create a "normal" board.
    // Setup occurs according to the rules given in the game.
    Board::Board& VanillaGameScenario(std::vector<int> roles,int _difficulty,bool verbose=false){
        Board::Board* new_board = new Board::Board(roles,_difficulty);
        new_board -> setup(verbose);
        return *new_board;
    }

    // A scenario designed to test whether the ForcedDiscardConstructor can return both event card and citycard discard actions
    Board::Board& ForcedDiscardScenario(bool verbose=false){
        // Quarantine Specialist, Scientist, Researcher @ difficulty 4
        Board::Board* new_board = new Board::Board({0,2,3},4);
        // Use existing setup utility just to seed some hands
        new_board -> setup(true);

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

        return *new_board;
    }

    // A scenario designed to have an agent try lots of non-movement actions
    Board::Board& BusyBoardTest(bool verbose){
        Board::Board* new_board = new Board::Board({0,2,3},4);

        // Put 6 research stations more-or-less next to eachother on the board somewhere;
        int random_city = rand() % Map::CITIES.size();

        // This doesn't make sense, but somehow arranging it this way actually forces the compiler to stop ignoring these commands
        new_board -> AddStation(Map::CITIES[random_city]);
        (*new_board).AddStation(Map::CITIES[(random_city+1) % Map::CITIES.size()]);
        (*new_board).AddStation(Map::CITIES[(random_city+2) % Map::CITIES.size()]);
        (*new_board).AddStation(Map::CITIES[(random_city+3) % Map::CITIES.size()]);
        (*new_board).AddStation(Map::CITIES[(random_city+4) % Map::CITIES.size()]);
        (*new_board).AddStation(Map::CITIES[(random_city+5) % Map::CITIES.size()]);

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
                for(Decks::PlayerCard& card: (*new_board).get_players()[p].hand){
                    DEBUG_MSG("[Scenarios::BusyBoardTest()] ... " << card.name << std::endl);
                }
            }
        }

        new_board -> is_setup();
        return *new_board;
    }

}

#endif