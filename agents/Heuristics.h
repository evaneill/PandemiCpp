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
    // Idea is to capture how close the loss is according to player-controllable parts of the game
    // MAX(exp(.5 * (outbreaks - 8)), MAX over diseases of (exp(4 * (cubes-25)/25)) for that disease))
    //
    // Both metrics are designed to get exponentially more worse the closer to game-losing status they are, and be relatively minor earlier in the game
    double LossProximity(Board::Board& game_board);

    // Compound heuristic measuring closeness to winning tempered by closeness to losing
    // = CureGoalConditionswStation * (1 - LossProximity)
    // Idea is that for any given initial state under considerations, maximum value of any children is capped by proximity to loss.
    // Agent is incentivized to pursue routes that maximize chance of winning while avoiding lots of badness
    double CompoundLossWin(Board::Board& game_board);
}

#endif