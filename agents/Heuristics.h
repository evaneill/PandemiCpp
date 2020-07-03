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

    // 0 = lose, 1 = win
    double PureGameReward(Board::Board& game_board);

    // .25 per disease cured
    double CureGoalHeuristic(Board::Board& game_board);

    // .25 per disease cured
    // + .2 * (max fraction of required cards required over all players) for each uncured disease
    double CureGoalConditions(Board::Board& game_board);

    // .25 per disease cured
    // + .15 * (max fraction of required cards required over all players) for each uncured disease
    // + .05 per each uncured disease where a player with required cards is at a research station
    // + .025 per each uncured disease where a player with required cards is next to a research station
    double CureGoalConditionswStation(Board::Board& game_board);

    // How close, in a coarse fashion, position is to losing. Can be used by itself and minimized, or compounded with other heuristics
    // Idea is to capture how close the loss is according to player-controllable parts of the game (outbreak count and disease count)
    //
    // Both metrics are designed to get worse the further into the game you are
    double LossProximity(Board::Board& game_board);

    // Like above but takes into account the presence of cities with 3 disease cubes specifically, since they may outbreak much later in the game but must typically be treated earlier
    // Treats outbreaks the same as above
    // Cube-count is weighted by the # present in a city
    double SmartLossProximity(Board::Board& game_board);

    // Compound heuristic putting together heuristics in the form
    // alpha * heuristic1 + (1-alpha) * heuristic2 (alpha = .5 by default)
    double CompoundHeuristic(Board::Board& game_board,double heuristic1(Board::Board& game_board), double heuristic2(Board::Board& game_board),double alpha=.5);
}

#endif