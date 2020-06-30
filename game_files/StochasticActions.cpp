#include <vector>
#include <array>

#include "StochasticActions.h"
#include "Actions.h"
#include "Board.h"
#include "Decks.h"
#include "Players.h"

// ===== DRAW FROM PLAYER DECK ACTION ===== (GAME LOGIC ONLY)
StochasticActions::PlayerCardDrawAction::PlayerCardDrawAction(int card):
    card_drawn(card)
    {
        movetype = "PLAYERDRAW";
    }

void StochasticActions::PlayerCardDrawAction::execute(Board::Board& game_board){

    game_board.updatePlayerDeck(card_drawn);

    Players::Player& active_player = game_board.active_player();

    if(!Decks::IS_EVENT(card_drawn) && !Decks::IS_EPIDEMIC(card_drawn)){
        // Insert the identical city card in their hand
        active_player.UpdateHand(card_drawn);
    } else if(Decks::IS_EVENT(card_drawn)){
        // Or insert the the event card
        active_player.UpdateHand(card_drawn);
    } else {
        game_board. broken() = true;
        game_board. broken_reasons().push_back("[StochasticActions::PlayerCardDrawAction()] drew unidentifiable card (index: " + std::to_string(card_drawn) + ", name: " + Decks::CARD_NAME(card_drawn) +  ", color: " +std::to_string(Decks::CARD_COLOR(card_drawn))+")");
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

std::string StochasticActions::PlayerCardDrawAction::repr(){
    return movetype + " drew " + Decks::CARD_NAME(card_drawn);
}

bool StochasticActions::PlayerCardDrawAction::legal(Board::Board& game_board){
    // Legal if it's the player's draw turn...
    if(game_board.get_turn_action()==4){
        // And only 0 or 1 player cards have been drawn so far
        if(game_board.get_player_cards_drawn()>=0 && game_board.get_player_cards_drawn()<=1){
            return true;
        } else {
            // This shouldn't be reachable
            game_board.broken()=true;
            game_board.broken_reasons().push_back("[StochasticActions::PlayerCardDrawAction()] It's Player Deck draw phase (4) but player_cards_drawn isn't 0 or 1");
        }
    }
    return false;
}
// =========================


// ===== EPIDEMIC ACTION ===== (GAME LOGIC ONLY)
StochasticActions::EpidemicDrawAction::EpidemicDrawAction(int _epidemic_card,int _infect_card_drawn):
    epidemic_card(_epidemic_card),
    infect_card_drawn(_infect_card_drawn)
    {
        movetype = "PLAYERDRAW: EPIDEMIC";
    }

void StochasticActions::EpidemicDrawAction::execute(Board::Board& game_board){
    
    strrep = "";
    
    bool quarantine_adjacent = false;

    // update this infect deck to reflect this card having been drawn from the bottom and used
    game_board.updateInfectDeck(infect_card_drawn,true);

    // update the player deck to reflect an epidemic having been drawn
    game_board.updatePlayerDeck(epidemic_card);

    // Check for existence & adjacency of quarantine specialist
    for(Players::Player& p: game_board.get_players()){
        if(p.role.quarantinespecialist){
            if(p.get_position()==infect_card_drawn){
                quarantine_adjacent=true;
            }
            for(int n: Map::CITY_NEIGHBORS(p.get_position())){
                if(infect_card_drawn==n){
                    quarantine_adjacent=true;
                }
            }
            break;
        }
    }
    // If there's either no player or there is but they're not adjacent, infect
    if(!quarantine_adjacent){
        std::array<int,2> outbreak_status = game_board.infect(infect_card_drawn,Decks::CARD_COLOR(infect_card_drawn),3);

        // Unlike with execute() want to make sure it's impossible to end up double-concatenating outbreak strings
        // Not sure if that could even happen but just in case.
        strrep +=" (caused " + std::to_string(outbreak_status[0]) + " total outbreak(s) - "+std::to_string(outbreak_status[1])+" blocked)";
    } else {
        strrep +=" (blocked by quarantine specialist!)";
    }

    game_board.readd_infect_discard();

    // If one player card was already drawn before this, then we've just drawn the second and we increment the game state
    if(game_board.get_player_cards_drawn()>=1){
        game_board.get_turn_action()++; // increment to infect draw phase
        game_board.get_player_cards_drawn()=0; // reset the count for next time around
    } else {
        // otherwise just increment and continue
        game_board.get_player_cards_drawn()++;
    }
}

std::string StochasticActions::EpidemicDrawAction::repr(){
    return movetype + " (drew " + Decks::CARD_NAME(infect_card_drawn) + " from bottom of infect deck)" + strrep;
}
// =========================

// ===== Player Card draw constructor =====

StochasticActions::PlayerDeckDrawActionConstructor::PlayerDeckDrawActionConstructor(){}

std::string StochasticActions::PlayerDeckDrawActionConstructor::get_movetype(){
    return movetype;
}

int StochasticActions::PlayerDeckDrawActionConstructor::n_actions(Board::Board& game_board){
    // # epidemics + num non-epidemics
    int output = game_board.remaining_player_cards();
    // subtract # of epidemics remaining in the deck
    output-=(game_board.difficulty - game_board.get_epidemic_count());
    if(game_board.epidemic_possible()){
        // If an epidemic is possible, can produce an action for any deck in the bottom-most group of the deck 
        return output+game_board.n_infect_cards(false);
    } else {
        // otherwise can only pull an infect card from the top group
        return output;
    }
}

Actions::Action* StochasticActions::PlayerDeckDrawActionConstructor::random_action(Board::Board& game_board){
    int card = game_board.draw_playerdeck_inplace();
    if(Decks::IS_EPIDEMIC(card)){
        int infected_city = game_board.draw_infectdeck_bottom_inplace();
        return new EpidemicDrawAction(card,infected_city);
    } else {
        return new PlayerCardDrawAction(card);
    }
}

std::vector<Actions::Action*> StochasticActions::PlayerDeckDrawActionConstructor::all_actions(Board::Board& game_board){
    // Unimplemented. Hypothetically just requires more board entrypoints to the deck
    return {}; 
}

bool StochasticActions::PlayerDeckDrawActionConstructor::legal(Board::Board& game_board){
    // Legal if it's the player's draw turn...
    if(game_board.get_turn_action()==4){
        // And only 0 or 1 player cards have been drawn so far
        if(game_board.get_player_cards_drawn()>=0 && game_board.get_player_cards_drawn()<=1){
            return true;
        } else {
            // This shouldn't be reachable
            game_board.broken()=true;
            game_board.broken_reasons().push_back("[StochasticActions::PlayerDeckDrawActionConstructor()] It's Player Deck draw phase (4) but player_cards_drawn isn't 0 or 1");
        }
    }
    return false;
}

// =======================

// ===== INFECT DECK DRAW ACTION ===== (GAME LOGIC ONLY)
StochasticActions::InfectDeckDrawAction::InfectDeckDrawAction(int card): 
    outbreak_track({0,0}),
    card_drawn(card){
        movetype = "INFECTDRAW";
    }

void StochasticActions::InfectDeckDrawAction::execute(Board::Board& game_board){
    // If it's not quiet night
    if(!game_board.quiet_night_status()){

        // Update infect deck to reflect this card having been drawn + used
        game_board.updateInfectDeck(card_drawn);

        // If this color hasn't been eradicated...
        if(!game_board.is_eradicated(Decks::CARD_COLOR(card_drawn))){

            // Check for existence & adjacency of quarantine specialist
            for(Players::Player& p: game_board.get_players()){
                if(p.role.quarantinespecialist){
                    if(p.get_position()==card_drawn){
                        QuarantineSpecialistBlocked=true;
                        break;
                    }
                    for(int n: Map::CITY_NEIGHBORS(p.get_position())){
                        if(card_drawn==n){
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
                outbreak_track = game_board.infect(card_drawn,Decks::CARD_COLOR(card_drawn),1);

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
            for(Players::Player& p : game_board.get_players()){
                p.reset_last_position();
            }
            
            // reset the next players "operations expert flight" boolean just in case
            game_board.active_player().used_OperationsExpertFlight=false;
        } else {
            // Otherwise just increment the number of infect cards drawn
            game_board.get_infect_cards_drawn()++;
        }
    } else {
        // No card will be removed from infect deck

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
        for(Players::Player& p : game_board.get_players()){
            p.reset_last_position();
        }

        // reset quiet_night status
        game_board.quiet_night_status()=false;
    }
}

std::string StochasticActions::InfectDeckDrawAction::repr(){
    // If there's either no player or there is but they're not adjacent, infect
    if(!QuarantineSpecialistBlocked){
        return movetype+ " infected " + Decks::CARD_NAME(card_drawn) + " (caused " + std::to_string(outbreak_track[0]) + " total outbreak(s) - "+std::to_string(outbreak_track[1])+" blocked)";
    } else {
        return movetype + " blocked by quarantine specialist";
    }
}
// =========================

// ===== INFECT draw action constructor =====

StochasticActions::InfectDeckDrawActionConstructor::InfectDeckDrawActionConstructor(){}

std::string StochasticActions::InfectDeckDrawActionConstructor::get_movetype(){
    return movetype;
}

int StochasticActions::InfectDeckDrawActionConstructor::n_actions(Board::Board& game_board){
    // number of infect cards in the top Group of cards on infect deck
    return game_board.n_infect_cards(true);
}

Actions::Action* StochasticActions::InfectDeckDrawActionConstructor::random_action(Board::Board& game_board){
    int card = game_board.draw_infectdeck_inplace();
    return new InfectDeckDrawAction(card);
}

std::vector<Actions::Action*> StochasticActions::InfectDeckDrawActionConstructor::all_actions(Board::Board& game_board){
    // Unimplemented. Hypothetically just requires more board entrypoints to the deck
    return {}; 
}

bool StochasticActions::InfectDeckDrawActionConstructor::legal(Board::Board& game_board){
    // Legal when it's the infect step and <Infection rate cards have been drawn.
    // (Quiet night consideration is within execute())
    if(game_board.get_turn_action()==5 && (game_board.get_infect_cards_drawn())<(game_board.get_infection_rate())){
        return true;
    } else {
        return false;
    }
}
// =========================

// ===== STOCHASTIC CONSTRUCTOR =====

StochasticActions::StochasticActionConstructor::StochasticActionConstructor():
    // don't think this is necessary?
    infect_draw_con(),
    player_draw_con(){}

bool StochasticActions::StochasticActionConstructor::legal(Board::Board& board){
    if(infect_draw_con.legal(board) || player_draw_con.legal(board)){
        return true;
    } else{
        return false;
    }
}

Actions::Action* StochasticActions::StochasticActionConstructor::get_action(Board::Board& game_board){
    if(player_draw_con.legal(game_board)){
        return player_draw_con.random_action(game_board);
    } else if(infect_draw_con.legal(game_board)){
        return infect_draw_con.random_action(game_board);
    } else {
        game_board.broken()=true;
        game_board.broken_reasons().push_back("[StochasticActions::StochasticActionConstructor()] Stochastic constructor was asked for actions when InfectDraw and PlayerDeckDraw were both illegal!");
        return nullptr;
    }
}

// =========================