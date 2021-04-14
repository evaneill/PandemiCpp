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

        bool quarantinespecialist= false;
        bool medic= false;
        bool scientist= false;
        bool researcher= false;
        bool operationsexpert= false;

        bool operator == (const Role& rhs){
            // fail-fast check (return false as soon as inequality observed)
            if(quarantinespecialist==rhs.quarantinespecialist){
                if(medic==rhs.medic){
                    if(scientist==rhs.scientist){
                        if(researcher==rhs.researcher){
                            if(operationsexpert==rhs.operationsexpert){
                                return true;
                            }
                        }
                    }
                }
            }
            return false;
        }
        bool operator != (const Role& rhs){
            return !(*this==rhs);
        }
    };

    class Player{
    protected:
        int hand_limit = 7;

        // player position by city index
        int position; 
    public:
        Player(int role_id);
        Player();

        ~Player(){
            // Do NOT delete the city
        };
        
        // reset for a new game (keep role but empty hand and put in atlanta, just like in constructor)
        void reset(); 
        Player(const Player& other){

            position = other.position;

            hand.clear();
            for(int c: other.hand){
                hand.push_back(c);
            }

            event_cards.clear();
            for(int e: other.event_cards){
                event_cards.push_back(e);
            }

            role = other.role;
            used_OperationsExpertFlight = other.used_OperationsExpertFlight;
        };

        Role role;

        std::vector<int> hand;// non-event cards
        std::vector<int> event_cards; // event cards

        // Update the non-event-card hand with a card denoted by index
        void UpdateHand(int drawn_card);

        // get copy of hand IN ORDER (for hashing right now)
        std::vector<int> get_all_cards();

        // Set the position to a new city
        void set_position(Map::City& new_city);
        void set_position(int new_city); // new_city index

        // Whether or not the player currently has _ABOVE_ hand limit
        bool hand_full();
        int handsize();

        // get rid of the event or city card matching the one in the argument by logical ==
        void removeCard(int card);
        // Get rid of the last required_cure_cards cards of color col in player hand (called during Cure::execute())
        void removeCureCardColor(int col);
        // Check number of each color of card they have
        std::array<int,4> get_color_count();

        // Return the city representing current position
        int get_position();

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