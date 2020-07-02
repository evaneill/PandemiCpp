#include "Heuristics.h"

#include <array>
#include <cmath>

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
    // Each uncured disease gets MAX .2 * ((# cards of this color)/(# needed to cure)) 
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
    for(Players::Player& p: game_board.get_players()){
        // find out how many of each color card they have
        std::array<int,4> color_count = p.get_color_count();
        
        // for each color, define "closeness" as min(1, (# cards of this color)/(# cards required to cure))
        if(!game_board.is_cured(Map::BLUE)){
            double pBlue_closeness = std::min((double) color_count[Map::BLUE]/(double) p.role.required_cure_cards,1.);
            if(pBlue_closeness > BLUE_closeness){
                BLUE_closeness = pBlue_closeness;
            }
        }

        if(!game_board.is_cured(Map::YELLOW)){
            double pYellow_closeness = std::min((double) color_count[Map::YELLOW]/(double) p.role.required_cure_cards,1.);
            if(pYellow_closeness > YELLOW_closeness){
                YELLOW_closeness = pYellow_closeness;
            }
        }

        if(!game_board.is_cured(Map::BLACK)){
            double pBlack_closeness = std::min((double) color_count[Map::BLACK]/(double) p.role.required_cure_cards,1.);
            if(pBlack_closeness > BLACK_closeness){
                BLACK_closeness = pBlack_closeness;
            }
        }

        if(!game_board.is_cured(Map::RED)){
            double pRed_closeness = std::min((double) color_count[Map::RED]/(double) p.role.required_cure_cards,1.);
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

double Heuristics::CureGoalConditionswStation(Board::Board& game_board){
    // Each cured disease gets 1/4 (like above)
    // Each uncured disease gets MAX .15 * ((# cards of this color)/(# needed to cure)) 
    //      (MAX is over players)
    // Each uncured disease gets an additional .05 if a player with ALL the necessary cards is at to a research station, OR
    //      .025 if a player with ALL the necessary cards is _next_ to a research station



    // heuristic ==1 IFF all diseases are cured
    // <1 otherwise.
    double value = 0;

    double BLUE_closeness=0; // max fraction of blue cards towards cure
    int BLUE_closest_position = -1; // position of player holding max fraction of blue cards needed

    double YELLOW_closeness=0; // max fraction of yellow cards towards cure
    int YELLOW_closest_position = -1; // position of player holding max fraction of yellow cards needed

    double BLACK_closeness=0; // max fraction of black cards towards cure
    int BLACK_closest_position = -1; // position of player holding max fraction of black cards needed

    double RED_closeness=0; // max fraction of red cards towards cure
    int RED_closest_position = -1; // position of player holding max fraction of red cards needed
    
    // Go through each player
    for(Players::Player& p: game_board.get_players()){
        // find out how many of each color card they have
        std::array<int,4> color_count = p.get_color_count();
        
        // for each color, define "closeness" as min(1, (# cards of this color)/(# cards required to cure))
        if(!game_board.is_cured(Map::BLUE)){
            double pBlue_closeness = std::min((double) color_count[Map::BLUE]/(double) p.role.required_cure_cards,1.);
            if(pBlue_closeness > BLUE_closeness){
                BLUE_closeness = pBlue_closeness;
                BLUE_closest_position = p.get_position();
            }
        }

        if(!game_board.is_cured(Map::YELLOW)){
            double pYellow_closeness = std::min((double) color_count[Map::YELLOW]/(double) p.role.required_cure_cards,1.);
            if(pYellow_closeness > YELLOW_closeness){
                YELLOW_closeness = pYellow_closeness;
                YELLOW_closest_position = p.get_position();
            }
        }

        if(!game_board.is_cured(Map::BLACK)){
            double pBlack_closeness = std::min((double) color_count[Map::BLACK]/(double) p.role.required_cure_cards,1.);
            if(pBlack_closeness > BLACK_closeness){
                BLACK_closeness = pBlack_closeness;
                BLACK_closest_position = p.get_position();
            }
        }

        if(!game_board.is_cured(Map::RED)){
            double pRed_closeness = std::min((double) color_count[Map::RED]/(double) p.role.required_cure_cards,1.);
            if(pRed_closeness > RED_closeness){
                RED_closeness = pRed_closeness;
                RED_closest_position = p.get_position();
            }
        }
    }

    // Add .05 for any disease for which all the cards are there based on whether the player is at/next to a research station
    if(BLUE_closeness==1){
        bool reward_claimed=false;
        for(int  st: game_board.get_stations()){
            if(BLUE_closest_position==st && !reward_claimed){
                value+=.05;
                reward_claimed=true;
                break;
            }
        }
        if(!reward_claimed){
            std::vector<int> neighbors = Map::CITY_NEIGHBORS(BLUE_closest_position);
            for(int& neighbor: neighbors){
                for(int  st : game_board.get_stations()){
                    if(st== neighbor && !reward_claimed){
                        value+=.025;
                        reward_claimed=true;
                        break;
                    }
                }
                if(reward_claimed){
                    break;
                }
            }
        }
    }
    if(YELLOW_closeness==1){
        bool reward_claimed=false;
        for(int  st: game_board.get_stations()){
            if(YELLOW_closest_position==st && !reward_claimed){
                value+=.05;
                reward_claimed=true;
                break;
            }
        }
        if(!reward_claimed){
            std::vector<int> neighbors = Map::CITY_NEIGHBORS(YELLOW_closest_position);
            for(int& neighbor: neighbors){
                for(int  st : game_board.get_stations()){
                    if(st== neighbor && !reward_claimed){
                        value+=.025;
                        reward_claimed=true;
                        break;
                    }
                }
                if(reward_claimed){
                    break;
                }
            }
        }
    }
    if(BLACK_closeness==1){
        bool reward_claimed=false;
        for(int  st: game_board.get_stations()){
            if(BLACK_closest_position==st && !reward_claimed){
                value+=.05;
                reward_claimed=true;
                break;
            }
        }
        if(!reward_claimed){
            std::vector<int> neighbors = Map::CITY_NEIGHBORS(BLACK_closest_position);
            for(int& neighbor: neighbors){
                for(int  st : game_board.get_stations()){
                    if(st== neighbor && !reward_claimed){
                        value+=.025;
                        reward_claimed=true;
                        break;
                    }
                }
                if(reward_claimed){
                    break;
                }
            }
        }
    }
    if(RED_closeness==1){
        bool reward_claimed=false;
        for(int  st: game_board.get_stations()){
            if(RED_closest_position==st && !reward_claimed){
                value+=.05;
                reward_claimed=true;
                break;
            }
        }
        if(!reward_claimed){
            std::vector<int> neighbors = Map::CITY_NEIGHBORS(RED_closest_position);
            for(int& neighbor: neighbors){
                for(int  st : game_board.get_stations()){
                    if(st== neighbor && !reward_claimed){
                        value+=.025;
                        reward_claimed=true;
                        break;
                    }
                }
                if(reward_claimed){
                    break;
                }
            }
        }
    }

    // Add either .25 or .15*closeness for each disease
    if(!game_board.is_cured(Map::BLUE)){
        value+=.15*BLUE_closeness;
    } else{
        value+=.25;
    }
    if(!game_board.is_cured(Map::YELLOW)){
        value+=.15*YELLOW_closeness;
    } else{
        value+=.25;
    }
    if(!game_board.is_cured(Map::BLACK)){
        value+=.15*BLACK_closeness;
    } else{
        value+=.25;
    }
    if(!game_board.is_cured(Map::RED)){
        value+=.15*RED_closeness;
    } else{
        value+=.25;
    }
    return value;
}

double Heuristics::LossProximity(Board::Board& game_board){
    // Made in anticipation of having a compound-heuristic agent, where it'd be weighted equally with a Cure Precondition heuristic
    //      Wanted .5 * (changes in this heuristic) to be comparable and maybe a bit less than changes in Cure Precondition heuristic.
    
    double outbreak_badness = 0;
    int outbreak_count = game_board.get_outbreak_count();
    //  outbreaks: 0,   1,      2,    3,    4,      5,     6,     7
    //        h(): 0,   0,    .04,  .08,  .12,    .21,   .30,   .39
    if(outbreak_count>=2 && outbreak_count<=4){
        outbreak_badness=.04 * ((double) outbreak_count - 1.);
    } else if(outbreak_count==5){
        outbreak_badness=.21;
    } else if(outbreak_count==6){
        outbreak_badness=.30;
    } else if(outbreak_count==7){
        outbreak_badness=.39;
    }
    
    std::array<int,4> color_count= game_board.get_color_count();

    // Piecewise linear fxn mostly 0 (<=18) then sloping up
    double BLUE_badness = std::max(0.,((double) color_count[Map::BLUE] - 18.)/32.);
    double YELLOW_badness = std::max(0.,((double) color_count[Map::YELLOW] - 18.)/32.);
    double BLACK_badness = std::max(0.,((double) color_count[Map::BLACK] - 18.)/32.);
    double RED_badness = std::max(0.,((double) color_count[Map::RED] - 18.)/32.);

    // Higher slope when count >21
    BLUE_badness = std::max(BLUE_badness,3.*((double) color_count[Map::BLUE] - 21.)/32. + 3./32.);
    YELLOW_badness = std::max(YELLOW_badness,3.*((double) color_count[Map::YELLOW] - 21.)/32. + 3./32.);
    BLACK_badness = std::max(BLACK_badness,3.*((double) color_count[Map::BLACK] - 21.)/32. + 3./32.);
    RED_badness = std::max(RED_badness,3.*((double) color_count[Map::RED] - 21.)/32. + 3./32.);

    double max_disease_badness = BLUE_badness;
    if(YELLOW_badness>max_disease_badness){
        max_disease_badness = YELLOW_badness;
    }
    if(BLACK_badness>max_disease_badness){
        max_disease_badness = BLACK_badness;
    }
    if(RED_badness>max_disease_badness){
        max_disease_badness = RED_badness;
    }

    // <1 always during a game (constructed so that max(outbreak_badness) ~ max(disease badness) < .5)
    return 1 - outbreak_badness - max_disease_badness;
}

double Heuristics::SmartLossProximity(Board::Board& game_board){
    // Goal is to account for both "badness" that's present on the board, and "badness" that's bound to happen in the future
    // Specifically want to fold in the fact that cities with 3 cubes are bad news for the future agent, ever if even a deep planner may not be able to recognize it.

    // ========= "Badness" associated to cities with 3 disease cubes =========
    std::array<std::array<int,48>,4> disease_count = game_board.get_disease_count();

    int BLUE_count = 0;
    for(int city=0;city<48;city++){
        // For cities where there's 3, check that it's possible it will ever be drawn again
        if(disease_count[Map::BLUE][city]==3){
            // if it's not the case that both the city has been drawn already and an epidemic is impossible,
            if(!(game_board.in_infect_discard(city) && !game_board.epidemic_possible())){
                // Then increment the count
                BLUE_count+=1;
                // Then check neighbors to see if there's a nominal risk of chain outbreak
                for(int neighbor: Map::CITY_NEIGHBORS(city)){
                    if(disease_count[Map::BLUE][neighbor]==3){
                        // Idea is that two cities next to eachother -> count = 3 = (1 + .5 + 1 + .5)
                        // three cities w/ 3 next to eachother - > count = 6 
                        BLUE_count+=.5;
                    }
                }
            } else {
                BLUE_count+=.5; // same value as 2-cube city
            }
        } else {
            // If not 3, we don't care about anything but adding a much smaller quantity to the count
            BLUE_count+= .25 * disease_count[Map::BLUE][city];
        }
    }

    int YELLOW_count = 0;
    for(int city=0;city<48;city++){
        // For cities where there's 3, check that it's possible it will ever be drawn again
        if(disease_count[Map::YELLOW][city]==3){
            // if it's not the case that both the city has been drawn already and an epidemic is impossible,
            if(!(game_board.in_infect_discard(city) && !game_board.epidemic_possible())){
                // Then increment the count
                YELLOW_count+=1;
                // Then check neighbors to see if there's a nominal risk of chain outbreak
                for(int neighbor: Map::CITY_NEIGHBORS(city)){
                    if(disease_count[Map::YELLOW][neighbor]==3){
                        // Idea is that two cities next to eachother -> count = 3 = (1 + .5 + 1 + .5)
                        // three cities w/ 3 next to eachother - > count = 6 
                        YELLOW_count+=.5;
                    }
                }
            } else {
                YELLOW_count+=.5; // same value as 2-cube city
            }
        } else {
            // If not 3, we don't care about anything but adding a much smaller quantity to the count
            YELLOW_count+= .25 * disease_count[Map::YELLOW][city];
        }
    }

    int BLACK_count = 0;
    for(int city=0;city<48;city++){
        // For cities where there's 3, check that it's possible it will ever be drawn again
        if(disease_count[Map::BLACK][city]==3){
            // if it's not the case that both the city has been drawn already and an epidemic is impossible,
            if(!(game_board.in_infect_discard(city) && !game_board.epidemic_possible())){
                // Then increment the count
                BLACK_count+=1;
                // Then check neighbors to see if there's a nominal risk of chain outbreak
                for(int neighbor: Map::CITY_NEIGHBORS(city)){
                    if(disease_count[Map::BLACK][neighbor]==3){
                        // Idea is that two cities next to eachother -> count = 3 = (1 + .5 + 1 + .5)
                        // three cities w/ 3 next to eachother - > count = 6 
                        BLACK_count+=.5;
                    }
                }
            } else {
                BLACK_count+=.5; // same value as 2-cube city
            }
        } else {
            // If not 3, we don't care about anything but adding a much smaller quantity to the count
            BLACK_count+= .25 * disease_count[Map::BLACK][city];
        }
    }

    int RED_count = 0;
    for(int city=0;city<48;city++){
        // For cities where there's 3, check that it's possible it will ever be drawn again
        if(disease_count[Map::RED][city]==3){
            // if it's not the case that both the city has been drawn already and an epidemic is impossible,
            if(!(game_board.in_infect_discard(city) && !game_board.epidemic_possible())){
                // Then increment the count
                RED_count+=1;
                // Then check neighbors to see if there's a nominal risk of chain outbreak
                for(int neighbor: Map::CITY_NEIGHBORS(city)){
                    if(disease_count[Map::RED][neighbor]==3){
                        // Idea is that two cities next to eachother -> count = 3 = (1 + .5 + 1 + .5)
                        // three cities w/ 3 next to eachother - > count = 6 
                        RED_count+=.5;
                    }
                }
            } else {
                RED_count+=.5; // same value as 2-cube city
            }
        } else {
            // If not 3, we don't care about anything but adding a much smaller quantity to the count
            RED_count+= .25 * disease_count[Map::RED][city];
        }
    }
    // Divide by 6 = value when 3 cities w/ 3 are all next to eachother 
    // (it can be worse than that, but rarely, and this would make it be appropriately hella bad anyway)
    // This means that badness for any one disease can be MORE than 1. I'm counting on the fact that this being the case for one disease would make it *not* the case for others, which is true in a vanilla game
    // 
    double BLUE_3cube_badness = BLUE_count/=6;
    double YELLOW_3cube_badness = YELLOW_count/=6.;
    double BLACK_3cube_badness = BLACK_count/=6.;
    double RED_3cube_badness = RED_count/=6;

    // I normalize the sum by 2 rather than 4 since I want to stretch typical values to span more of [0,1], and I feel like typically on average they're already < 1/2 each
    // Max with 0 *just in case*, since losing (score = 0 ) should NEVER be a more attractive option than a bad board state.
    return std::max(1. - (BLUE_3cube_badness + YELLOW_3cube_badness + BLACK_3cube_badness + RED_3cube_badness)/2,0.);
}

double Heuristics::CompoundHeuristic(Board::Board& game_board,double heuristic1(Board::Board& game_board), double heuristic2(Board::Board& game_board),double alpha){
    return alpha * heuristic1(game_board) + (1.-alpha)*heuristic2(game_board);
}