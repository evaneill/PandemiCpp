#ifndef PLAYER_H
#define PLAYER_H

#include <iostream>
#include <vector>
#include <string>
#include <array>

#include "Decks.h"
#include "Map.h"

namespace Players
{
    class Role{
    public:
        Role(std::string name,int req_cure_cards);
        Role();
        ~Role(){};
        std::string name;
        int required_cure_cards;// 5 for all but scientist (4)
    };

    class Player{
    protected:
        int hand_limit = 7;

        // Map::City position of player.
        Map::City position; 

        int last_position=-1; // Track last position to reduce search
    public:
        Player(int role_id);
        Player();

        ~Player(){};
        
        // reset for a new game (keep role but empty hand and put in atlanta, just like in constructor)
        void reset(); 
        Player(const Player& other){
            position = other.position;

            hand.clear();
            for(Decks::PlayerCard c: other.hand){
                hand.push_back(c);
            }

            event_cards.clear();
            for(Decks::PlayerCard e: other.event_cards){
                event_cards.push_back(e);
            }

            role = other.role;
            used_OperationsExpertFlight = other.used_OperationsExpertFlight;
        };

        Role role;

        std::vector<Decks::PlayerCard> hand;// non-event cards
        std::vector<Decks::PlayerCard> event_cards; // event cards

        // Update the non-event-card hand with a CityCard
        void UpdateHand(Decks::CityCard drawn_card);
        
        // Update the event-card hand with an EventCard
        void UpdateHand(Decks::EventCard drawn_card);

        // Update with either
        void UpdateHand(Decks::PlayerCard drawn_card);

        // Set the position to a new city
        void set_position(Map::City new_city);
        void set_position(int new_city); // new_city index
        void reset_last_position(int old_position=-1); // Change last_position tracker to old_position (by default remove tracking status)

        // Whether or not the player currently has _ABOVE_ hand limit
        bool hand_full();
        int handsize();

        // get rid of the event or city card matching the one in the argument by logical ==
        void removeCard(Decks::PlayerCard card);
        // Get rid of the last required_cure_cards cards of color col in player hand (called during Cure::execute())
        void removeCureCardColor(int col);
        // Check number of each color of card they have
        std::array<int,4> get_color_count();

        // Return the city representing current position
        Map::City get_position();
        int get_last_position();

        // To track whether or not they've jumped from a research station to somewhere else
        // <DANGER!> Relies on game logic to use properly ONLY for operations expert.
        bool used_OperationsExpertFlight;
    };

    class QuarantineSpecialist: public Role{
    public:
        QuarantineSpecialist();
        ~QuarantineSpecialist(){};
    };
    class Medic: public Role{
    public:
        Medic();
        ~Medic(){};
    };
    class Researcher: public Role{
    public:
        Researcher();
        ~Researcher(){};
    };
    class Scientist: public Role{
    public:
        Scientist();
        ~Scientist(){};
    };
    class OperationsExpert: public Role{
    public:
        OperationsExpert();
        ~OperationsExpert(){};
    };
};

#endif