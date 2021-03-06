#include <iostream>

#include "SanityCheck.h"
#include "Board.h"
#include "Map.h"
#include "Players.h"
#include "Debug.h"

#include <algorithm>
#include <string>
#include <array>

void SanityCheck::CheckBoard(Board::Board& active_board,bool verbose){

    // Designed to collect ALL badness instead of breaking and failing fast.

    // make sure all disease counts are >=0 and <=3
    if(verbose){
        DEBUG_MSG(std::endl << "[SANITYCHECK] Checking disease counts on each city..." << std::endl);
    }
    for(int col=0;col<4;col++){
        for(int city=0;city<Map::CITIES.size();city++){
            if(active_board.get_disease_count()[col][city]<0 || active_board.get_disease_count()[col][city]>3){
                if(verbose){
                    DEBUG_MSG("[SANITYCHECK] ... " << Map::CITIES[city].name << " has " << active_board.get_disease_count()[col][city] << " " << Map::COLORS[col] << " cubes on it! that's bad." << std::endl);
                }
                active_board.broken()=true;
                active_board.broken_reasons().push_back("[SANITYCHECK] " + Map::CITIES[city].name+ " has " + std::to_string(active_board.get_disease_count()[col][city]) + " " + Map::COLORS[col] +" cubes");
            }
        }
    }
    if(verbose){
        DEBUG_MSG("[SANITYCHECK] done!" << std::endl);
    }

    // make sure turn action is >=0 and <=5
    if(verbose){
        DEBUG_MSG(std::endl << "[SANITYCHECK] Checking that turn_action on board is well defined..." << std::endl);
    }
    if(active_board.get_turn_action()<0 || active_board.get_turn_action()>5){
        if(verbose){
            DEBUG_MSG(std::endl << "[SANITYCHECK] turn_action is ill defined: it's currently " << active_board.get_turn_action() << std::endl);
        }
        active_board.broken()=true;
        active_board.broken_reasons().push_back("[SANITYCHECK] turn_action is ill defined: it's currently " + std::to_string(active_board.get_turn_action()));
    }
    if(verbose){
        DEBUG_MSG("[SANITYCHECK] done!" << std::endl);
    }

    // make sure infect card drawn counters are 0 when it's not infect stage of the game
    if(verbose){
        DEBUG_MSG(std::endl << "[SANITYCHECK] Checking infection card drawn counter is 0 when not infect step..." << std::endl);
    }
    if(active_board.get_turn_action()<=4){
        if(active_board.get_infect_cards_drawn()>0){
            if(verbose){
                DEBUG_MSG("[SANITYCHECK] ... but on turn-action " << active_board.get_turn_action() << " the counter is >0!" << std::endl);
            }
            active_board.broken()=true;
            active_board.broken_reasons().push_back("[SANITYCHECK] infect_cards_drawn is "+std::to_string(active_board.get_infect_cards_drawn())+" but should be 0 since its stage + " + std::to_string(active_board.get_turn_action()) );
        }
    }
    if(verbose){
        DEBUG_MSG("[SANITYCHECK] done!" << std::endl);
    }

    // make sure player card draw is 0 outside of that stage
    if(verbose){
        DEBUG_MSG(std::endl << "[SANITYCHECK] Checking player cards drawn counter is 0 when not player draw step..." << std::endl);
    }
    if(active_board.get_turn_action()<=3 || active_board.get_turn_action()==5){
        if(active_board.get_player_cards_drawn()>0){
            if(verbose){
                DEBUG_MSG("[SANITYCHECK] ... but on turn-action " << active_board.get_turn_action() << " the counter is >0!" << std::endl);
            }
            active_board.broken()=true;
            active_board.broken_reasons().push_back("[SANITYCHECK] player_cards_drawn is "+std::to_string(active_board.get_player_cards_drawn())+" but should be 0 since its stage + " + std::to_string(active_board.get_turn_action()) );
        }
    }
    if(verbose){
        DEBUG_MSG("[SANITYCHECK] done!" << std::endl);
    }

    // make sure no player has duplicate cards (city or event)
    // The most costly part 4 sure
    if(verbose){
        DEBUG_MSG(std::endl << "[SANITYCHECK] Checking player hands for duplicate cards..." << std::endl);
    }
    for(Players::Player& p: active_board.get_players()){
        if(p.hand.size()>1){
            for(int c=0;c<(p.hand.size()-1);c++){
                for(int k=c+1;k<p.hand.size();k++){
                    if(p.hand[c]==p.hand[k]){
                        if(verbose){
                            DEBUG_MSG("[SANITYCHECK] ... player " << p.role.name << " has two of " << Decks::CARD_NAME(p.hand[c]) << "!" << std::endl);
                        }
                        active_board.broken()=true;
                        active_board.broken_reasons().push_back("[SANITYCHECK] " + p.role.name + " has two of the same city card: " + Decks::CARD_NAME(p.hand[c]) + " (card " + std::to_string(c) + ") and "+ Decks::CARD_NAME(p.hand[k]) + "(card "+std::to_string(k) + ")" );
                    }
                }
            }
        }
        if(p.event_cards.size()>1){
            for(int c=0;c<(p.event_cards.size()-1);c++){
                for(int k=c+1;k<p.event_cards.size();k++){
                    if(p.event_cards[c]==p.event_cards[k]){
                        if(verbose){
                            DEBUG_MSG("[SANITYCHECK] ... player " << p.role.name << " has two of " << Decks::CARD_NAME(p.event_cards[c]) << "!" << std::endl);
                        }
                        active_board.broken()=true;
                        active_board.broken_reasons().push_back("[SANITYCHECK] " + p.role.name + " has two of the same event card: " + Decks::CARD_NAME(p.event_cards[c]) + " (event card " + std::to_string(c) + ") and "+ Decks::CARD_NAME(p.event_cards[k]) + "(event card "+std::to_string(k) + ")" );
                    }
                }
            }
        }
    }
    if(verbose){
        DEBUG_MSG("[SANITYCHECK] done!" << std::endl);
    }

    // Check that all player hands are at most size 8
    // Size 8 is allowed given that ForcedDiscard is _supposed_ to be called whenever this is the case
    if(verbose){
        DEBUG_MSG(std::endl <<  "[SANITYCHECK] Checking all players have <=8 cards..." << std::endl);
    }
    for(Players::Player& p: active_board.get_players()){
        if(p.handsize()>8){
            if(verbose){
                DEBUG_MSG("... but " << p.role.name << " has " << p.handsize() << " cards!" << std::endl);
            }
            active_board.broken()=true;
            active_board.broken_reasons().push_back("[SANITYCHECK] " + p.role.name+" has " +std::to_string(p.handsize()) + " cards! ("+std::to_string(p.hand.size())+" city cards and " +std::to_string(p.event_cards.size()) + " event cards)");
        }
    }
    if(verbose){
        DEBUG_MSG("[SANITYCHECK] done!" << std::endl);
    }

    // Epidemics drawn should be 0<=epidemics <= difficulty
    if(verbose){
        DEBUG_MSG(std::endl <<  "[SANITYCHECK] Checking that there are <=difficulty epidemic cards drawn" << std::endl);
    }
    if(active_board.get_epidemic_count()<0 || active_board.get_epidemic_count()>active_board.get_difficulty()){
        if(verbose){
            DEBUG_MSG("[SANITYCHECK] ... but there are " << active_board.get_epidemic_count() << " epidemics, even though difficulty is " << active_board.get_difficulty() << std::endl);
        }
        active_board.broken()=true;
        active_board.broken_reasons().push_back("[SANITYCHECK] Difficulty is " + std::to_string(active_board.get_difficulty()) + " and "+ std::to_string(active_board.get_epidemic_count())+" epidemics have been drawn");
    }
    if(verbose){
        DEBUG_MSG("[SANITYCHECK] done!" << std::endl);
    }

    // Make sure disease_sum is returning same value as the actual disease tracking array disease_count
    // (this is a very suboptimal sanity check, but my experiments show that even with it, keeping a separate integer tracker reaps small rewards)
    if(verbose){
        DEBUG_MSG(std::endl <<  "[SANITYCHECK] Checking that integer disease count tracker is the same as the stored disease counts for each city and color" << std::endl);
    }
    if(active_board.disease_sum(Map::BLUE)!=std::accumulate(active_board.get_disease_count()[Map::BLUE].begin(),active_board.get_disease_count()[Map::BLUE].end(),0)){
        if(verbose){
            DEBUG_MSG("[SANITYCHECK] " << Map::COLORS[Map::BLUE] << " disease_count sum isn't the same as its tracker!");
        }
        active_board.broken()=true;
        active_board.broken_reasons().push_back("[SANITYCHECK] " +Map::COLORS[Map::BLUE] + " disease_count sum isn't the same as its tracker!");
    }
    if(active_board.disease_sum(Map::YELLOW)!=std::accumulate(active_board.get_disease_count()[Map::YELLOW].begin(),active_board.get_disease_count()[Map::YELLOW].end(),0)){
        if(verbose){
            DEBUG_MSG("[SANITYCHECK] " << Map::COLORS[Map::YELLOW] << " disease_count sum isn't the same as its tracker!");
        }
        active_board.broken()=true;
        active_board.broken_reasons().push_back("[SANITYCHECK] " +Map::COLORS[Map::YELLOW] + " disease_count sum isn't the same as its tracker!");
    }
    if(active_board.disease_sum(Map::BLACK)!=std::accumulate(active_board.get_disease_count()[Map::BLACK].begin(),active_board.get_disease_count()[Map::BLACK].end(),0)){
        if(verbose){
            DEBUG_MSG("[SANITYCHECK] " << Map::COLORS[Map::BLACK] << " disease_count sum isn't the same as its tracker!");
        }
        active_board.broken()=true;
        active_board.broken_reasons().push_back("[SANITYCHECK] " +Map::COLORS[Map::BLACK] + " disease_count sum isn't the same as its tracker!");
    }
    if(active_board.disease_sum(Map::RED)!=std::accumulate(active_board.get_disease_count()[Map::RED].begin(),active_board.get_disease_count()[Map::RED].end(),0)){
        if(verbose){
            DEBUG_MSG("[SANITYCHECK] " << Map::COLORS[Map::RED] << " disease_count sum isn't the same as its tracker!");
        }
        active_board.broken()=true;
        active_board.broken_reasons().push_back("[SANITYCHECK] " +Map::COLORS[Map::RED] + " disease_count sum isn't the same as its tracker!");
    }
    if(verbose){
        DEBUG_MSG("[SANITYCHECK] done!" << std::endl);
    }

    // Make sure disease_sum is returning same value as the actual disease tracking array disease_count
    // (this is a very suboptimal sanity check, but my experiments show that even with it, keeping a separate integer tracker reaps small rewards)
    if(verbose){
        DEBUG_MSG(std::endl <<  "[SANITYCHECK] Checking that every player position is 0<= position <= 48" << std::endl);
    }
    for(Players::Player& p: active_board.get_players()){
        if(p.get_position()<0 || p.get_position()>48){
            active_board.broken()=true;
            active_board.broken_reasons().push_back("[SANITYCHECK] "+ p.role.name + " has position listed as " + std::to_string(p.get_position()) + "!");
        }
    }
    if(verbose){
        DEBUG_MSG("[SANITYCHECK] done!" << std::endl);
    }
}