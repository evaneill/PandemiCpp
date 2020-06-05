#ifndef HEURISTICS_H
#define HEURISTICS_H

#include <array>

#include "../game_files/Board.h"
#include "../game_files/Map.h"
#include "../game_files/Players.h"

// contains maps Board::Board -> double to be used by either game logic or agents
namespace Heuristics
{
    typedef double Heuristic (Board::Board&);

    double PureGameReward(Board::Board& game_board);

    double CureGoalHeuristic(Board::Board& game_board);

    double CureGoalConditions(Board::Board& game_board);
}

#endif