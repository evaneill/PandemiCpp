#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <array>

#include "Players.h"
#include "Map.h"
#include "Debug.h"

Players::Player::Player(int role_id): 
    position(3),
    hand({}),
    used_OperationsExpertFlight(false)
    {
    // Make it work for now with a hard-coded reference to roles
    // Should only matter at game instantiation! So get it write once in an agent test and it's always right.
    switch(role_id){
        case 0:
            role = Players::QuarantineSpecialist();
            break;
        case 1:
            role = Players::Medic();
            break;
        case 2:
            role = Players::Scientist();
            break;
        case 3:
            role = Players::Researcher();
            break;
        case 4:
            role = Players::OperationsExpert();
            break;
        default:
            role = Role(NULL,0); // Should break the game quickly on a bad input
    }
}

Players::Player::Player(){
    position=-1;
    hand = {};
    used_OperationsExpertFlight=false;
    role = Role(NULL,0);
}

void Players::Player::reset(){
    // put them back in hotlanta
    position = 3;
    hand.clear();
    event_cards.clear();
    used_OperationsExpertFlight=false;
}

// Supposed to tell when a player is _ABOVE_ hand limit
bool Players::Player::hand_full(){
    return (hand.size()+event_cards.size())>hand_limit;
}

void Players::Player::UpdateHand(int drawn_card){
    if(!Decks::IS_EVENT(drawn_card) && !Decks::IS_EPIDEMIC(drawn_card)){
        hand.push_back(drawn_card);
    } else if(Decks::IS_EVENT(drawn_card)){
        event_cards.push_back(drawn_card);
    } else {
        DEBUG_MSG(std::endl << "[Player::UpdateHand()] [" << role.name << "] did not add card " << Decks::CARD_NAME(drawn_card) << "(index " << drawn_card << ")!!!!" << std::endl);
    }
}

std::vector<int> Players::Player::get_all_cards(){
    std::vector<int> output = {};
    output = hand;
    for(int c : event_cards){
        output.push_back(c);
    }
    std::sort(output.begin(),output.end());
    return output;
}

int Players::Player::handsize(){
    return hand.size()+event_cards.size();
}

std::array<int,4> Players::Player::get_color_count(){
    int BLUE_count=0,YELLOW_count=0,BLACK_count=0,RED_count=0;
    for(int& card: hand){
        switch(Decks::CARD_COLOR(card)){
            case Map::BLUE:
                BLUE_count++;
                break;
            case Map::YELLOW:
                YELLOW_count++;
                break;
            case Map::BLACK:
                BLACK_count++;
                break;
            case Map::RED:
                RED_count++;
                break;
            default:
                DEBUG_MSG("[Player::get_color_count()] Encountered a card in " << role.name << " hand with no color! (color: " << Decks::CARD_COLOR(card) << ", name: " << Decks::CARD_NAME(card) << ")" <<std::endl);
                break;
        }
    }
    return {BLUE_count,YELLOW_count,BLACK_count,RED_count};
}

void Players::Player::set_position(Map::City& new_city){
    position = new_city.index;
};

void Players::Player::set_position(int new_city){
    position = new_city;
};

void Players::Player::removeCard(int card_to_remove){
    // copying int for the smallest possible optimization - the value isn't used but to decide where to delete
    int hand_idx=0;
    for(int& card:hand){
        if(card==card_to_remove){
            hand.erase(hand.begin()+hand_idx);
            return;
        }
        hand_idx++;
    }
    hand_idx=0;
    for(int& card:event_cards){
        if(card==card_to_remove){
            event_cards.erase(event_cards.begin()+hand_idx);
            return;
        }
        hand_idx++;
    }
}

void Players::Player::removeCureCardColor(int col){
    int checkpoint = 0;
    int num_erased=0;
    while(num_erased<role.required_cure_cards){
        // Erase card indices from 
        if(Decks::CARD_COLOR(hand[checkpoint])==col){
            hand.erase(hand.begin()+checkpoint);
            num_erased++;
        } else{
            checkpoint++;
        }
    }
}

int Players::Player::get_position(){
    return position;
}

Players::Role::Role(std::string _name,int req_cure_cards){
    name = _name;
    required_cure_cards = req_cure_cards;
}
Players::Role::Role(){};

Players::QuarantineSpecialist::QuarantineSpecialist(): Role("Quarantine Specialist",5){
    quarantinespecialist=true;
};
Players::Medic::Medic(): Role("Medic",5){
    medic=true;
};
Players::Scientist::Scientist(): Role("Scientist",4){
    scientist=true;
};
Players::Researcher::Researcher(): Role("Researcher",5){
    researcher=true;
};
Players::OperationsExpert::OperationsExpert(): Role("Operations Expert",5){
    operationsexpert=true;
};