#include <iostream>
#include <vector>
#include <string>
#include <cassert>

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
    if(drawn_card.index<Map::CITIES.size()){
        hand.push_back(drawn_card);
    } else if(drawn_card.index>=Map::CITIES.size()){
        event_cards.push_back(drawn_card);
    } else {
        DEBUG_MSG(std::endl << "[Player::UpdateHand] [" << role.name << "] did not add card " << drawn_card.name << "(index " << drawn_card.index << ")!!!!" << std::endl);
    }
}

int Players::Player::handsize(){
    return hand.size()+event_cards.size();
}

void Players::Player::set_position(Map::City new_city){
    position = new_city;
};

void Players::Player::set_position(int new_city){
    position = Map::CITIES[new_city];
};

void Players::Player::removeCard(Decks::PlayerCard card_to_remove){
    for(int c=0;c<hand.size();c++){
        if(hand[c].index==card_to_remove.index){
            hand.erase(hand.begin()+c);
            return;
        }
    }
    for(int c=0;c<event_cards.size();c++){
        if(event_cards[c].index==card_to_remove.index){
            event_cards.erase(event_cards.begin()+c);
            return;
        }
    }
}

Map::City Players::Player::get_position(){
    return position;
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