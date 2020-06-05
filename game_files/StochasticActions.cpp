#include <vector>
#include <array>

#include "StochasticActions.h"
#include "Actions.h"
#include "Board.h"
#include "Decks.h"
#include "Players.h"

// ===== DRAW FROM PLAYER DECK ACTION ===== (GAME LOGIC ONLY)
StochasticActions::PlayerDeckDrawAction::PlayerDeckDrawAction()
    {
        movetype = "PLAYERDRAW";
    }

void StochasticActions::PlayerDeckDrawAction::execute(Board::Board& game_board){
    
    // update the game status if we've taken out the last card. Do nothing and lose if called on an empty player deck
    if(!game_board.player_deck_nonempty()){
        game_board.has_lost()=true;
        game_board.get_lost_reason() = "Ran out of player cards!";
    } else {
        Players::Player& active_player = game_board.active_player();

        // Get a new card
        Decks::PlayerCard new_card = game_board.draw_playerdeck();

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
            Decks::InfectCard new_card = game_board.draw_infectdeck_bottom();
            
            card_drawn += " (" + new_card.name + " got drawn from bottom of infect deck)";

            bool quarantine_adjacent = false;

            // Check for existence & adjacency of quarantine specialist
            for(Players::Player* p: game_board.get_players()){
                if((*p).role.name=="Quarantine Specialist"){
                    if((*p).get_position().index==new_card.index){
                        quarantine_adjacent=true;
                    }
                    for(int n: (*p).get_position().neighbors){
                        if(new_card.index==n){
                            quarantine_adjacent=true;
                        }
                    }
                    break;
                }
            }
            // If there's either no player or there is but they're not adjacent, infect
            if(!quarantine_adjacent){
                std::array<int,2> outbreak_status = game_board.infect(new_card.index,new_card.color,3);

                // Unlike with execute() want to make sure it's impossible to end up double-concatenating outbreak strings
                // Not sure if that could even happen but just in case.
                card_drawn +=" (caused " + std::to_string(outbreak_status[0]) + " total outbreak(s) - "+std::to_string(outbreak_status[1])+" blocked)";
            } else {
                card_drawn +=" (blocked by quarantine specialist!)";
            }

            game_board.readd_infect_discard();
        } else {
            game_board. broken() = true;
            game_board. broken_reasons().push_back("[StochasticActions::PlayerDeckDraw()] drew unidentifiable card (index: " + std::to_string(new_card.index) + ", name: " + new_card.name + ", color: " +std::to_string(new_card.color)+")");
        }
        // If one player card was already drawn before this, then we've just drawn the second and we increment the game state
        if(game_board.get_player_cards_drawn()>=1){
            game_board.get_turn_action()++; // increment to infect draw phase
            game_board.get_player_cards_drawn()=0; // reset the count for next time around
        } else {
            // otherwise just increment and continue
            game_board.get_player_cards_drawn()++;
        }
    }
}

std::string StochasticActions::PlayerDeckDrawAction::repr(){
    return movetype + " drew " + card_drawn;
}

bool StochasticActions::PlayerDeckDrawAction::legal(Board::Board& game_board){
    // Legal if it's the player's draw turn...
    if(game_board.get_turn_action()==4){
        // And only 0 or 1 player cards have been drawn so far
        if(game_board.get_player_cards_drawn()>=0 && game_board.get_player_cards_drawn()<=1){
            return true;
        } else {
            // This shouldn't be reachable
            game_board.broken()=true;
            game_board.broken_reasons().push_back("It's Player Deck draw phase (4) but player_cards_drawn isn't 0 or 1");
        }
    }
    return false;
}
// =========================

// ===== INFECT DECK DRAW ACTION ===== (GAME LOGIC ONLY)
StochasticActions::InfectDeckDrawAction::InfectDeckDrawAction(): 
    outbreak_track({0,0}){
        movetype = "INFECTDRAW";
    }

void StochasticActions::InfectDeckDrawAction::execute(Board::Board& game_board){
    // If it's not quiet night
    if(!game_board.quiet_night_status()){
        // draw a card
        Decks::InfectCard new_card = game_board.draw_infectdeck();

        // Set card name for the repr()
        card_drawn = new_card.name;

        // If this color hasn't been eradicated...
        if(!game_board.is_eradicated(new_card.color)){

            // Check for existence & adjacency of quarantine specialist
            for(Players::Player* p: game_board.get_players()){
                if(typeid((*p).role)==typeid(Players::QuarantineSpecialist)){
                    if((*p).get_position().index==new_card.index){
                        QuarantineSpecialistBlocked=true;
                        break;
                    }
                    for(int n: (*p).get_position().neighbors){
                        if(new_card.index==n){
                            QuarantineSpecialistBlocked=true;
                        }
                    }
                    break;
                }
            }
            // If there's either no player or there is but they're not adjacent, infect
            if(!QuarantineSpecialistBlocked){
                // little bit questionable having outbreak_track incremented by an execution on a different board.
                // Hypothetically shouldn't be an issue
                outbreak_track = game_board.infect(new_card.index,new_card.color,1);

                // check for losing status on number of outbreaks & disease cube counts
                if(!game_board.outbreak_count_safe()){
                    game_board.has_lost()=true;
                    game_board.get_lost_reason() = "Have >7 outbreaks!";
                } else if(!game_board.disease_count_safe()){
                    game_board.has_lost()=true;
                    game_board.get_lost_reason() = "Have used >24 disease cubes of some color!";
                }
            } 
        }
        // If we've drawn enough cards, move the board on to prepare for the next player round if we've drawn all the infect cards
        if((game_board.get_infect_cards_drawn())>=(game_board.get_infection_rate()-1)){
            // reset to player turn ("action 0")
            game_board.get_turn_action() = 0; 

            // reset # infect cards drawn for next round
            game_board.get_infect_cards_drawn() = 0; 

            // Increment the player turn by 1 (in a cyclic way)
            game_board.get_turn() = (game_board.get_turn() + 1) % game_board.get_players().size();

            // Reset player position memory
            for(Players::Player* p : game_board.get_players()){
                (*p).reset_last_position();
            }
            
            // reset the next players "operations expert flight" boolean just in case
            game_board.active_player().used_OperationsExpertFlight=false;
        } else {
            // Otherwise just increment the number of infect cards drawn
            game_board.get_infect_cards_drawn()++;
        }
    } else {
        // Just skip the entire infect turn
        // reset to player turn ("action 0")
        game_board.get_turn_action() = 0; 

        // reset # infect cards drawn for next round
        game_board.get_infect_cards_drawn() = 0; 

        // Increment the player turn by 1 (in a cyclic way)
        game_board.get_turn() = (game_board.get_turn() + 1) % game_board.get_players().size();

        // reset the next players "operations expert flight" boolean just in case
        game_board.active_player().used_OperationsExpertFlight=false;

        // Reset player position memory
        for(Players::Player* p : game_board. get_players()){
            (*p).reset_last_position();
        }

        game_board.quiet_night_status()=false;
    }
}

std::string StochasticActions::InfectDeckDrawAction::repr(){
    // If there's either no player or there is but they're not adjacent, infect
    if(!QuarantineSpecialistBlocked){
        return movetype+ " infected " + card_drawn + " (caused " + std::to_string(outbreak_track[0]) + " total outbreak(s) - "+std::to_string(outbreak_track[1])+" blocked)";
    } else {
        return movetype + " blocked by quarantine specialist at " + card_drawn;
    }
}

bool StochasticActions::InfectDeckDrawAction::legal(Board::Board& game_board){
    // Legal when it's the infect step and <Infection rate cards have been drawn.
    if(game_board.get_turn_action()==5 && (game_board.get_infect_cards_drawn())<(game_board.get_infection_rate())){
        return true;
    } else {
        return false;
    }
}

// =========================

// ===== STOCHASTIC CONSTRUCTOR =====

StochasticActions::StochasticActionConstructor::StochasticActionConstructor(){
    infect_draw = InfectDeckDrawAction();
    player_draw = PlayerDeckDrawAction();
}

bool StochasticActions::StochasticActionConstructor::legal(Board::Board& board){
    if(infect_draw.legal(board) || player_draw.legal(board)){
        return true;
    } else{
        return false;
    }
}

Actions::Action* StochasticActions::StochasticActionConstructor::get_action(Board::Board& game_board){
    if(player_draw.legal(game_board)){
        return new StochasticActions::PlayerDeckDrawAction();
    } else if(infect_draw.legal(game_board)){
        return new StochasticActions::InfectDeckDrawAction();
    } else {
        game_board.broken()=true;
        game_board.broken_reasons().push_back("Stochastic constructor was asked for actions when InfectDraw and PlayerDeckDraw were both illegal!");
    }
}

// =========================