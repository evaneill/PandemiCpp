#ifndef HASHES_H
#define HASHES_H

#include "../../game_files/Board.h"
#include "../../game_files/Decks.h"
#include "../../game_files/Players.h"

namespace Hashes
{
    // Hash the board, which requires...
    size_t boardHash(Board::Board& board);

    // a hash of the 
    size_t playerHash(const std::vector<Players::Player>& players);

    // and a hash of the infect deck
    size_t infectDeckHash(const Decks::InfectDeck& infect_deck);

    // and a hash of all the little components
    size_t VecHash(const std::vector<int>& vec);
    size_t VecHash(const std::vector<bool>& vec);
    size_t ArrayArrayHash(const std::array<std::array<int,48>,4>& multiarray);
}

#endif