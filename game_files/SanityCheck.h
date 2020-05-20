#ifndef SANITY_CHECK_H
#define SANITY_CHECK_H

#include "Board.h"

namespace SanityCheck{
    // Goal of this function is just to update BROKEN status of board if the board is broke
    // Want to concentrate as many logical checks here as possible
    void CheckBoard(Board::Board& active_board);
}

#endif