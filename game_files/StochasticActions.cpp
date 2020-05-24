#include <vector>
#include <array>

#include "StochasticActions.h"
#include "Actions.h"
#include "Board.h"
#include "Decks.h"
#include "Players.h"

// ===== DRAW FROM PLAYER DECK ACTION ===== (GAME LOGIC ONLY)
StochasticActions::PlayerDeckDrawAction::PlayerDeckDrawAction(Board::Board& _active_board):
    Action(_active_board){
        movetype = "PLAYERDRAW";
    }

void StochasticActions::PlayerDeckDrawAction::execute(){
    Players::Player& active_player = active_board ->active_player();

    // Get a new card
    Decks::PlayerCard new_card = active_board ->draw_playerdeck();
    // assign the string for repr()
    card_drawn = new_card.name;

    if(!new_card.event && !new_card.epidemic){
        // Insert the identical city card in their hand
        active_player.UpdateHand(Decks::CityCard(new_card.index));
    } else if(new_card.event){
        // Or insert the the event card
        active_player.UpdateHand(Decks::EventCard(new_card.index));
    } else if(new_card.epidemic){
        // This must be an epidemic card
        // Logic of how many have been drawn has already been updated at this point in the deck
        Decks::InfectCard new_card = active_board -> draw_infectdeck_bottom();
        
        card_drawn += " (" + new_card.name + " got drawn from bottom of infect deck)";

        bool quarantine_adjacent = false;

        // Check for existence & adjacency of quarantine specialist
        for(Players::Player& p: active_board ->get_players()){
            if(p.role.name=="Quarantine Specialist"){
                if(p.get_position().index==new_card.index){
                    quarantine_adjacent=true;
                }
                for(int n: p.get_position().neighbors){
                    if(new_card.index==n){
                        quarantine_adjacent=true;
                    }
                }
                break;
            }
        }
        // If there's either no player or there is but they're not adjacent, infect
        if(!quarantine_adjacent){
            std::array<int,2> outbreak_status = active_board ->infect(new_card.index,new_card.color,3);
            card_drawn += " (caused " + std::to_string(outbreak_status[0]) + " total outbreak(s) - "+std::to_string(outbreak_status[1])+" blocked)";
        }

        active_board ->readd_infect_discard();
    } else {
        active_board -> broken() = true;
        active_board -> broken_reasons().push_back("[StochasticActions::PlayerDeckDraw()] drew unidentifiable card (index: " + std::to_string(new_card.index) + ", name: " + new_card.name + ", color: " +std::to_string(new_card.color)+")");
    }
    // If one player card was already drawn before this, then we've just drawn the second and we increment the game state
    if(active_board ->get_player_cards_drawn()>=1){
        active_board ->get_turn_action()++; // increment to infect draw phase
        active_board ->get_player_cards_drawn()=0; // reset the count for next time around
    } else {
        // otherwise just increment and continue
        active_board ->get_player_cards_drawn()++;
    }
    // update the game status if we've taken out the last card
    if(!active_board ->player_deck_nonempty()){
        active_board ->has_lost()=true;
        active_board ->get_lost_reason() = "Ran out of player cards!";
    }
}

std::string StochasticActions::PlayerDeckDrawAction::repr(){
    return movetype + " drew " + card_drawn;
}

bool StochasticActions::PlayerDeckDrawAction::legal(){
    // Legal if it's the player's draw turn...
    if(active_board ->get_turn_action()==4){
        // And only 0 or 1 player cards have been drawn so far
        if(active_board ->get_player_cards_drawn()>=0 && active_board ->get_player_cards_drawn()<=1){
            return true;
        } else {
            // This shouldn't be reachable
            active_board ->broken()=true;
            active_board ->broken_reasons().push_back("It's Player Deck draw phase (4) but player_cards_drawn isn't 0 or 1");
        }
    }
    return false;
}
// =========================

// ===== INFECT DECK DRAW ACTION ===== (GAME LOGIC ONLY)
StochasticActions::InfectDrawAction::InfectDrawAction(Board::Board& _active_board): 
    Action(_active_board),
    outbreak_track({0,0}){
        movetype = "INFECTDRAW";
    }

void StochasticActions::InfectDrawAction::execute(){
    // If it's not quiet night
    if(!(active_board ->quiet_night_status())){
        // draw a card
        Decks::InfectCard new_card = active_board ->draw_infectdeck();

        // Set card name for the repr()
        card_drawn = new_card.name;

        // If this color hasn't been eradicated...
        if(!active_board ->get_eradicated()[new_card.color]){

            // Check for existence & adjacency of quarantine specialist
            for(Players::Player& p: active_board ->get_players()){
                if(typeid(p.role)==typeid(Players::QuarantineSpecialist)){
                    if(p.get_position().index==new_card.index){
                        QuarantineSpecialistBlocked=true;
                    }
                    for(int n: p.get_position().neighbors){
                        if(new_card.index==n){
                            QuarantineSpecialistBlocked=true;
                        }
                    }
                    break;
                }
            }
            // If there's either no player or there is but they're not adjacent, infect
            if(!QuarantineSpecialistBlocked){
                outbreak_track = active_board ->infect(new_card.index,new_card.color,1);

                // check for losing status on number of outbreaks & disease cube counts
                if(!active_board ->outbreak_count_safe()){
                    active_board ->has_lost()=true;
                    active_board -> get_lost_reason() = "Have >7 outbreaks!";
                } else if(!active_board ->disease_count_safe()){
                    active_board ->has_lost()=true;
                    active_board -> get_lost_reason() = "Have used >24 disease cubes of some color!";
                }
            } 

            // If we've drawn enough cards, move the board on to prepare for the next player round if we've drawn all the infect cards
            if((active_board ->get_infect_cards_drawn())>=(active_board ->get_infection_rate()-1)){
                // reset to player turn ("action 0")
                active_board ->get_turn_action() = 0; 

                // reset # infect cards drawn for next round
                active_board ->get_infect_cards_drawn() = 0; 

                // Increment the player turn by 1 (in a cyclic way)
                active_board ->get_turn() = (active_board ->get_turn() + 1) % active_board ->get_players().size();

                // reset the next players "operations expert flight" boolean just in case
                active_board ->active_player().used_OperationsExpertFlight=false;
            } else {
                // Otherwise just increment the number of infect cards drawn
                active_board -> get_infect_cards_drawn()++;
            }
        }
    } else {
        // Just skip the entire infect turn
        // reset to player turn ("action 0")
        active_board ->get_turn_action() = 0; 

        // reset # infect cards drawn for next round
        active_board ->get_infect_cards_drawn() = 0; 

        // Increment the player turn by 1 (in a cyclic way)
        active_board ->get_turn() = (active_board ->get_turn() + 1) % active_board ->get_players().size();

        // reset the next players "operations expert flight" boolean just in case
        active_board ->active_player().used_OperationsExpertFlight=false;

        active_board ->quiet_night_status()=false;
    }
}

std::string StochasticActions::InfectDrawAction::repr(){
    // If there's either no player or there is but they're not adjacent, infect
    if(!QuarantineSpecialistBlocked){
        return movetype+ " infected " + card_drawn + " (caused " + std::to_string(outbreak_track[0]) + " total outbreak(s) - "+std::to_string(outbreak_track[1])+" blocked)";
    } else {
        return movetype + " blocked by quarantine specialist at " + card_drawn;
    }
}

bool StochasticActions::InfectDrawAction::legal(){
    // Legal when it's the infect step and <Infection rate cards have been drawn.
    if(active_board ->get_turn_action()==5 && (active_board ->get_infect_cards_drawn())<(active_board ->get_infection_rate())){
        return true;
    } else {
        return false;
    }
}

// =========================

// ===== STOCHASTIC CONSTRUCTOR =====

StochasticActions::StochasticActionConstructor::StochasticActionConstructor(Board::Board& _active_board){
        active_board = &_active_board;
}

bool StochasticActions::StochasticActionConstructor::legal(){
    if(StochasticActions::InfectDrawAction(*active_board).legal() || StochasticActions::PlayerDeckDrawAction(*active_board).legal()){
        return true;
    } else{
        return false;
    }
}

Actions::Action* StochasticActions::StochasticActionConstructor::get_action(){
    if(StochasticActions::PlayerDeckDrawAction(*active_board).legal()){
        return new StochasticActions::PlayerDeckDrawAction(*active_board);
    } else if(StochasticActions::InfectDrawAction(*active_board).legal()){
        return new StochasticActions::InfectDrawAction(*active_board);
    } else {
        active_board -> broken()=true;
        active_board -> broken_reasons().push_back("Stochastic constructor was asked for actions when InfectDraw and PlayerDeckDraw were both illegal!");
    }
}


// =========================
