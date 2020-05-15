#ifndef PLAYER_H
#define PLAYER_H

#include <iostream>
#include <vector>
#include <string>

#include "Decks.h"
#include "Map.h"

namespace Players
{
    class Role{
    public:
        Role(std::string name,int req_cure_cards);
        Role();
        std::string name;
        int required_cure_cards;// 5 for all but scientist (4)
    };

    class Player{
    public:
        Player(int role_id,Map::Cities set_map);

        Map::City position; // position on map
        Role role;
        std::vector<Decks::PlayerCard> hand;
        
    };

    class QuarantineSpecialist: public Role{
    public:
        QuarantineSpecialist();
    };
    class Medic: public Role{
    public:
        Medic();
    };
    class Researcher: public Role{
    public:
        Researcher();
    };
    class Scientist: public Role{
    public:
        Scientist();
    };
    class OperationsExpert: public Role{
    public:
        OperationsExpert();
    };
};

#endif