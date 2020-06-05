#include "Heuristics.h"

#include <array>

#include "../game_files/Board.h"
#include "../game_files/Map.h"
#include "../game_files/Players.h"

double Heuristics::PureGameReward(Board::Board& game_board){
    // Only care about the W/L status of a game
    return (double) game_board.win_lose();
}

double Heuristics::CureGoalHeuristic(Board::Board& game_board){
    // *Only* cares about how many diseases are cured -> (number of cured diseases)/4
    // Equivalent of # disjoint subgoals required to win
    return ((double) game_board.is_cured(Map::BLUE)+
        (double) game_board.is_cured(Map::YELLOW)+
        (double) game_board.is_cured(Map::BLACK)+
        (double) game_board.is_cured(Map::RED))/4;
};

double Heuristics::CureGoalConditions(Board::Board& game_board){
    // Each cured disease gets 1/4 (like above)
    // Each uncured disease gets MAX .2 * ( - (# cards of this color)/(# needed to cure)) 
    //      (MAX is over players)
    // Idea is that at most, value given to a set of players for an uncured disease is .2 (less than the .25 they could have by curing too)

    // heuristic ==1 IFF all diseases are cured
    // <1 otherwise.
    double value = 0;

    double BLUE_closeness=0;
    double YELLOW_closeness=0;
    double BLACK_closeness=0;
    double RED_closeness=0;
    
    // Go through each player
    for(Players::Player* p: game_board.get_players()){
        // find out how many of each color card they have
        std::array<int,4> color_count = (*p).get_color_count();
        
        // for each color, define "closeness" as min(1, (# cards of this color)/(# cards required to cure))
        if(!game_board.is_cured(Map::BLUE)){
            double pBlue_closeness = std::min((double) color_count[Map::BLUE]/(double) (*p).role.required_cure_cards,1.);
            if(pBlue_closeness > BLUE_closeness){
                BLUE_closeness = pBlue_closeness;
            }
        }

        if(!game_board.is_cured(Map::YELLOW)){
            double pYellow_closeness = std::min((double) color_count[Map::YELLOW]/(double) (*p).role.required_cure_cards,1.);
            if(pYellow_closeness > YELLOW_closeness){
                YELLOW_closeness = pYellow_closeness;
            }
        }

        if(!game_board.is_cured(Map::BLACK)){
            double pBlack_closeness = std::min((double) color_count[Map::BLACK]/(double) (*p).role.required_cure_cards,1.);
            if(pBlack_closeness > BLACK_closeness){
                BLACK_closeness = pBlack_closeness;
            }
        }

        if(!game_board.is_cured(Map::RED)){
            double pRed_closeness = std::min((double) color_count[Map::RED]/(double) (*p).role.required_cure_cards,1.);
            if(pRed_closeness > RED_closeness){
                RED_closeness = pRed_closeness;
            }
        }
    }
    if(!game_board.is_cured(Map::BLUE)){
        value+=.2*BLUE_closeness;
    } else{
        value+=.25;
    }
    if(!game_board.is_cured(Map::YELLOW)){
        value+=.2*YELLOW_closeness;
    } else{
        value+=.25;
    }
    if(!game_board.is_cured(Map::BLACK)){
        value+=.2*BLACK_closeness;
    } else{
        value+=.25;
    }
    if(!game_board.is_cured(Map::RED)){
        value+=.2*RED_closeness;
    } else{
        value+=.25;
    }
    return value;
};