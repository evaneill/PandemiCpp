#include "SanityCheck.h"
#include "Board.h"
#include "Map.h"
#include "Players.h"

#include <algorithm>
#include <string>

void SanityCheck::CheckBoard(Board::Board& active_board){

    // Designed to collect ALL badness instead of breaking and failing fast.

    // make sure all disease counts are >=0 and <=3
    for(int col=0;col<4;col++){
        for(int city=0;city<Map::CITIES.size();city++){
            if(active_board.get_disease_count()[col][city]<0 || active_board.get_disease_count()[col][city]>3){
                active_board.broken()=true;
                active_board.broken_reasons().push_back(Map::CITIES[city].name+ " has " + std::to_string(active_board.get_disease_count()[col][city]) + " " + Map::COLORS[col] +" cubes");
            }
        }
    }

    // make sure infect card drawn counters are 0 when it's not infect stage of the game
    if(active_board.get_turn_action()<=4){
        if(active_board.get_infect_cards_drawn()>0){
            active_board.broken()=true;
            active_board.broken_reasons().push_back("infect_cards_drawn is "+std::to_string(active_board.get_infect_cards_drawn())+" but should be 0 since its stage + " + std::to_string(active_board.get_turn_action()) );
        }
    }

    // make sure player card draw is 0 outside of that stage
    if(active_board.get_turn_action()<=3 || active_board.get_turn_action()==5){
        if(active_board.get_player_cards_drawn()>0){
            active_board.broken()=true;
            active_board.broken_reasons().push_back("player_cards_drawn is "+std::to_string(active_board.get_player_cards_drawn())+" but should be 0 since its stage + " + std::to_string(active_board.get_turn_action()) );
        }
    }

    // make sure no player has duplicate cards (city or event)
    // The most costly part 4 sure
    for(Players::Player& p: active_board.get_players()){
        if(p.hand.size()>0){
            for(int c=0;c<(p.hand.size()-1);c++){
                for(int k=c+1;k<p.hand.size();k++){
                    if(p.hand[c].index==p.hand[k].index){
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
                        active_board.broken()=true;
                        active_board.broken_reasons().push_back(p.role.name + " has two of the same event card: " + p.hand[c].name + " (event card " + std::to_string(c) + ") and "+ p.hand[k].name + "(event card "+std::to_string(k) + ")" );
                    }
                }
            }
        }
    }

    // Check that all player hands are at most size 8
    // Size 8 is allowed given that ForcedDiscard is _supposed_ to be called whenever this is the case
    for(Players::Player& p: active_board.get_players()){
        if(p.handsize()>8){
            active_board.broken()=true;
            active_board.broken_reasons().push_back(p.role.name+" has " +std::to_string(p.handsize()) + " cards! ("+std::to_string(p.hand.size())+" city cards and " +std::to_string(p.event_cards.size()) + " event cards)");
        }
    }

    // Epidemics drawn should be 0<=epidemics <= difficulty
    if(active_board.get_epidemic_count()<0 || active_board.get_epidemic_count()>active_board.get_difficulty()){
        active_board.broken()=true;
        active_board.broken_reasons().push_back("Difficulty is " + std::to_string(active_board.get_difficulty()) + " and "+ std::to_string(active_board.get_epidemic_count())+" epidemics have been drawn");
    }
}