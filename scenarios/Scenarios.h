#ifndef SCENARIOS_H
#define SCENARIOS_H

#include "../game_files/Board.h"

#include "../game_files/Decks.h"
#include "../game_files/Players.h"

#include "../game_files/Debug.h"

namespace Scenarios
{
    // This is the simplest example: Use the existing setup() method and create a "normal" board.
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
        // for(int c;c<10;c++){
        new_board -> active_player().UpdateHand(Decks::PlayerCard(50));
        new_board -> active_player().UpdateHand(Decks::PlayerCard(50));
        new_board -> active_player().UpdateHand(Decks::PlayerCard(50));
        new_board -> active_player().UpdateHand(Decks::PlayerCard(50));
        new_board -> active_player().UpdateHand(Decks::PlayerCard(50));
        new_board -> active_player().UpdateHand(Decks::PlayerCard(50));
        new_board -> active_player().UpdateHand(Decks::PlayerCard(50));
        // }
        for(Decks::PlayerCard& card: new_board -> active_player().event_cards){
            DEBUG_MSG(std::endl << "[Scenarios::ForcedDiscardScenario()] "<< new_board -> active_player().role.name << " has event card: " << card.name);
        }

        return *new_board;
    }

}

#endif