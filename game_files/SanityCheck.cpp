#include <iostream>

#include "SanityCheck.h"
#include "Board.h"
#include "Map.h"
#include "Players.h"

#include <algorithm>
#include <string>

void SanityCheck::CheckBoard(Board::Board& active_board,bool verbose){

    // Designed to collect ALL badness instead of breaking and failing fast.

    // make sure all disease counts are >=0 and <=3
    // std::cout << std::endl << "Checking disease counts on each city..." << std::endl;
    // for(int col=0;col<4;col++){
    //     for(int city=0;city<Map::CITIES.size();city++){
    //         if(active_board.get_disease_count()[col][city]<0 || active_board.get_disease_count()[col][city]>3){
    //             if(verbose){
    //                 std::cout << "... " << Map::CITIES[city].name << " has " << active_board.get_disease_count()[col][city] << " " << Map::COLORS[col] << " cubes on it! that's bad." << std::endl;
    //             }
    //             active_board.broken()=true;
    //             active_board.broken_reasons().push_back(Map::CITIES[city].name+ " has " + std::to_string(active_board.get_disease_count()[col][city]) + " " + Map::COLORS[col] +" cubes");
    //         }
    //     }
    // }
    // if(verbose){
    //     std::cout << "done!" << std::endl;
    // }

    // make sure infect card drawn counters are 0 when it's not infect stage of the game
    std::cout << std::endl << "Checking infection card drawn counter is 0 when not infect step..." << std::endl;
    if(active_board.get_turn_action()<=4){
        if(active_board.get_infect_cards_drawn()>0){
            if(verbose){
                std::cout << "... but on turn-action " << active_board.get_turn_action() << " the counter is >0!" << std::endl;
            }
            active_board.broken()=true;
            active_board.broken_reasons().push_back("infect_cards_drawn is "+std::to_string(active_board.get_infect_cards_drawn())+" but should be 0 since its stage + " + std::to_string(active_board.get_turn_action()) );
        }
    }
    if(verbose){
        std::cout << "done!" << std::endl;
    }

    // make sure player card draw is 0 outside of that stage
    std::cout << std::endl << "Checking player cards drawn counter is 0 when not player draw step..." << std::endl;
    if(active_board.get_turn_action()<=3 || active_board.get_turn_action()==5){
        if(active_board.get_player_cards_drawn()>0){
            if(verbose){
                std::cout << "... but on turn-action " << active_board.get_turn_action() << " the counter is >0!" << std::endl;
            }
            active_board.broken()=true;
            active_board.broken_reasons().push_back("player_cards_drawn is "+std::to_string(active_board.get_player_cards_drawn())+" but should be 0 since its stage + " + std::to_string(active_board.get_turn_action()) );
        }
    }
    if(verbose){
        std::cout << "done!" << std::endl;
    }

    // make sure no player has duplicate cards (city or event)
    // The most costly part 4 sure
    std::cout << std::endl << "About to check player hands for duplicate cards..." << std::endl;
    for(Players::Player& p: active_board.get_players()){
        if(p.hand.size()>0){
            for(int c=0;c<(p.hand.size()-1);c++){
                for(int k=c+1;k<p.hand.size();k++){
                    if(p.hand[c].index==p.hand[k].index){
                        if(verbose){
                            std::cout << "... player " << p.role.name << " has two of " << p.hand[c].name << "!" << std::endl;
                        }
                        active_board.broken()=true;
                        active_board.broken_reasons().push_back(p.role.name + " has two of the same city card: " + p.hand[c].name + " (card " + std::to_string(c) + ") and "+ p.hand[k].name + "(card "+std::to_string(k) + ")" );
                    }
                }
            }
        }
        if(p.event_cards.size()>0){
            for(int c=0;c<(p.event_cards.size()-1);c++){
                for(int k=c+1;k<p.event_cards.size();k++){
                    if(p.hand[c].index==p.hand[k].index){
                        if(verbose){
                            std::cout << "... player " << p.role.name << " has two of " << p.hand[c].name << "!" << std::endl;
                        }
                        active_board.broken()=true;
                        active_board.broken_reasons().push_back(p.role.name + " has two of the same event card: " + p.hand[c].name + " (event card " + std::to_string(c) + ") and "+ p.hand[k].name + "(event card "+std::to_string(k) + ")" );
                    }
                }
            }
        }
    }
    if(verbose){
        std::cout << "done!" << std::endl;
    }

    // Check that all player hands are at most size 8
    // Size 8 is allowed given that ForcedDiscard is _supposed_ to be called whenever this is the case
    std::cout << std::endl <<  "About to check all players have <=8 cards..." << std::endl;
    for(Players::Player& p: active_board.get_players()){
        if(p.handsize()>8){
            if(verbose){
                std::cout << "... but " << p.role.name << " has " << p.handsize() << " cards!" << std::endl;
            }
            active_board.broken()=true;
            active_board.broken_reasons().push_back(p.role.name+" has " +std::to_string(p.handsize()) + " cards! ("+std::to_string(p.hand.size())+" city cards and " +std::to_string(p.event_cards.size()) + " event cards)");
        }
    }
    if(verbose){
        std::cout << "done!" << std::endl;
    }

    // Epidemics drawn should be 0<=epidemics <= difficulty
    std::cout << std::endl <<  "Making sure there are <=difficulty epidemic cards drawn" << std::endl;
    if(active_board.get_epidemic_count()<0 || active_board.get_epidemic_count()>active_board.get_difficulty()){
        if(verbose){
            std::cout << "... but there are " << active_board.get_epidemic_count() << " epidemics, even though difficulty is " << active_board.get_difficulty() << std::endl;
        }
        active_board.broken()=true;
        active_board.broken_reasons().push_back("Difficulty is " + std::to_string(active_board.get_difficulty()) + " and "+ std::to_string(active_board.get_epidemic_count())+" epidemics have been drawn");
    }
    if(verbose){
        std::cout << "done!" << std::endl;
    }
    
}