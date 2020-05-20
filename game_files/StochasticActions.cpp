#include <vector>
#include <array>

#include "StochasticActions.h"
#include "Actions.h"
#include "Board.h"
#include "Decks.h"
#include "Players.h"

// ===== DRAW FROM PLAYER DECK ACTION ===== (GAME LOGIC ONLY)
StochasticActions::PlayerDeckDrawAction::PlayerDeckDrawAction(Board::Board& _active_board):
    StochasticAction(_active_board){
        movetype = "PLAYERDRAW";
    }

void StochasticActions::PlayerDeckDrawAction::execute(){
    Players::Player& active_player = active_board.active_player();

    // Get a new card
    Decks::PlayerCard new_card = active_board.draw_playerdeck();
    card_drawn = new_card.name;
    if(typeid(new_card)==typeid(Decks::CityCard)){
        // Insert the identical city card in their hand
        active_player.UpdateHand(Decks::CityCard(new_card.index));
    } else if(typeid(new_card)==typeid(Decks::EventCard)){
        // Or insert the identical 
        switch(new_card.index){
            case 48:
                active_player.UpdateHand(Decks::EventCard(48,"Quiet Night"));
                break;
            case 49:
                active_player.UpdateHand(Decks::EventCard(49,"Government Grant"));
                break;
            case 50:
                active_player.UpdateHand(Decks::EventCard(50,"Airlift"));
                break;
            default:
                active_board.broken()=true;
                active_board.broken_reasons().push_back("Card index drawn during player deck draw was " + std::to_string(new_card.index) + " (name: " + card_drawn+ " )");
                break;
        }
    } else if(typeid(new_card)==typeid(Decks::EpidemicCard)){
        Decks::InfectCard new_card = active_board.draw_infectdeck_bottom();
        
        // Increment epidemics drawn, and so value returned by called get_infection_rate()
        active_board.get_epidemic_count()++;

        bool quarantine_adjacent = false;

        // Check for existence & adjacency of quarantine specialist
        for(Players::Player& p: active_board.get_players()){
            if(typeid(p.role)==typeid(Players::QuarantineSpecialist)){
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
            active_board.infect_city(new_card,3);
        }

        active_board.readd_infect_discard();
    }
    // If one player card was already drawn before this, then we've just drawn the second and we increment the game state
    if(active_board.get_player_cards_drawn()>=1){
        active_board.get_turn_action()++; // increment to infect draw phase
        active_board.get_player_cards_drawn()=0; // reset the count for next time around
    } else {
        // otherwise just increment and continue
        active_board.get_player_cards_drawn()++;
    }
    // update the game status if we've taken out the last card
    if(!active_board.player_deck_nonempty()){
        active_board.has_lost()=true;
    }
}

std::string StochasticActions::PlayerDeckDrawAction::repr(){
    return movetype + " drew " + card_drawn;
}

bool StochasticActions::PlayerDeckDrawAction::legal(){
    if(active_board.get_turn_action()==4){
        if(active_board.get_player_cards_drawn()>=0 && active_board.get_player_cards_drawn()<=1){
            return true;
        } else {
            // This shouldn't be reachable
            active_board.broken()=true;
            active_board.broken_reasons().push_back("It's Player Deck draw phase (4) but player_cards_drawn isn't 0 or 1");
        }
    }
    return false;
}
// =========================

// ===== INFECT DECK DRAW ACTION ===== (GAME LOGIC ONLY)
StochasticActions::InfectDrawAction::InfectDrawAction(Board::Board& _active_board): 
    StochasticAction(_active_board),
    outbreak_track({0,0}){
        movetype = "INFECTDRAW";
    }

void StochasticActions::InfectDrawAction::execute(){
    // If it's not quiet night
    if(!active_board.quiet_night_status()){
        // draw a card
        Decks::InfectCard new_card = active_board.draw_infectdeck();

        // If this color hasn't been eradicated...
        if(!active_board.get_eradicated()[new_card.color]){

            // Check for existence & adjacency of quarantine specialist
            for(Players::Player& p: active_board.get_players()){
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
                outbreak_track = active_board.infect_city(new_card,1);

                // check for losing status on number of outbreaks & disease cube counts
                if(!active_board.outbreak_count_safe() || !active_board.disease_count_safe()){
                    active_board.has_lost()=true;
                }
            } 

            // move the board on to prepare for the next player round if we've drawn all the infect cards
            if(active_board.get_infect_cards_drawn()>=(active_board.get_infection_rate()-1)){
                // reset to player turn ("action 0")
                active_board.get_turn_action() = 0; 

                // reset # infect cards drawn for next round
                active_board.get_infect_cards_drawn() = 0; 

                // Increment the player turn by 1 (in a cyclic way)
                active_board.get_turn() = (active_board.get_turn() + 1) % active_board.get_players().size();

                // reset the next players "operations expert flight" boolean just in case
                active_board.active_player().used_OperationsExpertFlight=false;
            }
        }
    } else {
        // Just skip the entire infect turn
        // reset to player turn ("action 0")
        active_board.get_turn_action() = 0; 

        // reset # infect cards drawn for next round
        active_board.get_infect_cards_drawn() = 0; 

        // Increment the player turn by 1 (in a cyclic way)
        active_board.get_turn() = (active_board.get_turn() + 1) % active_board.get_players().size();

        // reset the next players "operations expert flight" boolean just in case
        active_board.active_player().used_OperationsExpertFlight=false;
    }
    active_board.quiet_night_status()=false;
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
    if(active_board.get_turn_action()==5 && active_board.get_infect_cards_drawn()<active_board.get_infection_rate()){
        return true;
    } else {
        return false;
    }
}

// =========================

// ===== STOCHASTIC CONSTRUCTOR =====

StochasticActions::StochasticActionConstructor::StochasticActionConstructor(Board::Board& _active_board):
    ActionConstructor(_active_board){}

bool StochasticActions::StochasticActionConstructor::legal(){
    if(StochasticActions::InfectDrawAction(active_board).legal() || StochasticActions::PlayerDeckDrawAction(active_board).legal()){
        return true;
    } else{
        return false;
    }
}

Actions::Action StochasticActions::StochasticActionConstructor::get_action(){
    if(StochasticActions::PlayerDeckDrawAction(active_board).legal()){
        return StochasticActions::PlayerDeckDrawAction(active_board);
    } else if(StochasticActions::InfectDrawAction(active_board).legal()){
        return StochasticActions::InfectDrawAction(active_board);
    }
}


// =========================
