#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <array>

#include "Players.h"
#include "Map.h"
#include "Debug.h"

Players::Player::Player(int role_id): 
    position(Map::CITIES[3]),
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

bool Players::Player::hand_full(){
    return (hand.size()+event_cards.size())>hand_limit;
}

void Players::Player::UpdateHand(Decks::CityCard drawn_card){
    hand.push_back(drawn_card);
}

void Players::Player::UpdateHand(Decks::EventCard drawn_card){
    event_cards.push_back(drawn_card);
}

void Players::Player::UpdateHand(Decks::PlayerCard drawn_card){
    if(!drawn_card.event && !drawn_card.epidemic){
        hand.push_back(Decks::CityCard(drawn_card.index));
    } else if(drawn_card.event){
        event_cards.push_back(Decks::EventCard(drawn_card.index));
    } else {
        DEBUG_MSG(std::endl << "[Player::UpdateHand()] [" << role.name << "] did not add card " << drawn_card.name << "(index " << drawn_card.index << ")!!!!" << std::endl);
    }
}

int Players::Player::handsize(){
    return hand.size()+event_cards.size();
}

std::array<int,4> Players::Player::get_color_count(){
    int BLUE_count=0,YELLOW_count=0,BLACK_count=0,RED_count=0;
    for(Decks::PlayerCard& card: hand){
        switch(card.color){
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
                DEBUG_MSG("[Player::get_color_count()] Encountered a card in " << role.name << " hand with no color! (color: " << card.color << ", name: " << card.name << ")" <<std::endl);
                break;
        }
    }
    return {BLUE_count,YELLOW_count,BLACK_count,RED_count};
}

void Players::Player::set_position(Map::City new_city){
    position = new_city;
};

void Players::Player::set_position(int new_city){
    position = Map::CITIES[new_city];
};

void Players::Player::reset_last_position(int old_position){
    last_position=old_position;
};

void Players::Player::removeCard(Decks::PlayerCard card_to_remove){
    int hand_idx=0;
    for(Decks::PlayerCard& card:hand){
        if(card.index==card_to_remove.index){
            hand.erase(hand.begin()+hand_idx);
            return;
        }
        hand_idx++;
    }
    hand_idx=0;
    for(Decks::PlayerCard& card:event_cards){
        if(card.index==card_to_remove.index){
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
        if(hand[checkpoint].color==col){
            hand.erase(hand.begin()+checkpoint);
            num_erased++;
        } else{
            checkpoint++;
        }
    }
}

Map::City Players::Player::get_position(){
    return position;
}

int Players::Player::get_last_position(){
    return last_position;
}

Players::Role::Role(std::string _name,int req_cure_cards){
    name = _name;
    required_cure_cards = req_cure_cards;
}
Players::Role::Role(){};

Players::QuarantineSpecialist::QuarantineSpecialist(): Role("Quarantine Specialist",5){};
Players::Medic::Medic(): Role("Medic",5){};
Players::Scientist::Scientist(): Role("Scientist",4){};
Players::Researcher::Researcher(): Role("Researcher",5){};
Players::OperationsExpert::OperationsExpert(): Role("Operations Expert",5){};