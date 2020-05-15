#include <iostream>
#include <vector>
#include <string>

#include "Players.h"
#include "Map.h"

Players::Player::Player(int role_id,Map::Cities set_map): position(set_map.get_city(3)), hand({}) {
    // Make it work for now with a hard-coded reference to roles
    // Should only matter at game instantiation! So get it write once in an agent test and it's always right.
    switch(role_id){
        case 0:
            role = (Players::Role) QuarantineSpecialist();
            break;
        case 1:
            role = (Players::Role) Medic();
            break;
        case 2:
            role = (Players::Role) Scientist();
            break;
        case 3:
            role = (Players::Role) Researcher();
            break;
        case 4:
            role = (Players::Role) OperationsExpert();
            break;
        default:
            role = Role(NULL,0); // Should break the game quickly on a bad input
    }
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
Players::OperationsExpert::OperationsExpert(): Role("Operations Specialist",5){};