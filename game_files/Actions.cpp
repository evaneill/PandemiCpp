#include <iostream> // just for temp debugging I swear

#include <string>
#include <vector>
#include <algorithm>
#include <numeric>
#include <array>

#include "Actions.h"
#include "Board.h"
#include "Map.h"
#include "Players.h"

// ==== Pure Utility functions =====

// Neighbor test: is index 1 a neighbor of index 2?
bool isneighbor(int city_idx1, int city_idx2){
    // neighbors of city_idx2
    std::vector<int>& neighbors = Map::CITIES[city_idx2].neighbors;

    for(int& neighbor: neighbors){
        // If a neighbor is city_idx1, then...
        if(neighbor==city_idx1){
            return true;
        }
    }
    return false;
}

void update_medic_position(Board::Board& game_board){
    for(Players::Player& p: game_board.get_players()){
        // If any player is the medic
        if(p.role.medic){
            if(game_board.is_cured(Map::BLUE)){
                std::array<int,4>& color_count = game_board.get_color_count();
                std::array<std::array<int,48>,4>& disease_count = game_board.get_disease_count();
                // disease count of this color on their position should be 0
                color_count[Map::BLUE]-=disease_count[Map::BLUE][p.get_position()];
                disease_count[Map::BLUE][p.get_position()]=0;
            }
            if(game_board.is_cured(Map::YELLOW)){
                std::array<int,4>& color_count = game_board.get_color_count();
                std::array<std::array<int,48>,4>& disease_count = game_board.get_disease_count();
                // disease count of this color on their position should be 0
                color_count[Map::YELLOW]-=disease_count[Map::YELLOW][p.get_position()];
                disease_count[Map::YELLOW][p.get_position()]=0;
            }
            if(game_board.is_cured(Map::BLACK)){
                std::array<int,4>& color_count = game_board.get_color_count();
                std::array<std::array<int,48>,4>& disease_count = game_board.get_disease_count();
                // disease count of this color on their position should be 0
                color_count[Map::BLACK]-=disease_count[Map::BLACK][p.get_position()];
                disease_count[Map::BLACK][p.get_position()]=0;
            }
            if(game_board.is_cured(Map::RED)){
                std::array<int,4>& color_count = game_board.get_color_count();
                std::array<std::array<int,48>,4>& disease_count = game_board.get_disease_count();
                // disease count of this color on their position should be 0
                color_count[Map::RED]-=disease_count[Map::RED][p.get_position()];
                disease_count[Map::RED][p.get_position()]=0;
            }
        }
    }
}
// ========================

// ===== MOVE =====
Actions::Move::Move(int _to): to(_to){
    movetype = "MOVE";
}


void Actions::Move::execute(Board::Board& new_board){
    Players::Player& active_player = new_board.active_player();

    active_player.set_position(to);

    new_board.get_turn_action()++;

    new_board.LastAction_Move=true;

    // Make sure disease is cleared from medic position, if there
    update_medic_position(new_board);
}

std::string Actions::Move::repr(){
    return movetype+" to " + Map::CITY_NAME(to);
}

Actions::MoveConstructor::MoveConstructor(){}

std::string Actions::MoveConstructor::get_movetype(){
    return movetype;
}

int Actions::MoveConstructor::n_actions(Board::Board& game_board){
    if(legal(game_board)){
        return Map::CITY_NEIGHBORS(game_board.active_player().get_position()).size();
    } else{
        return 0;
    }
}

Actions::Action* Actions::MoveConstructor::random_action(Board::Board& game_board){
    Players::Player& active_player = game_board.active_player();
    std::vector<int> neighbors = Map::CITY_NEIGHBORS(active_player.get_position());
    int neighbor_idx = rand() % neighbors.size();
    // randomize a neighboring city index to move to it
    return new Actions::Move(neighbors[neighbor_idx]);
}

std::vector<Actions::Action*> Actions::MoveConstructor::all_actions(Board::Board& game_board){
    std::vector<int> neighbors = Map::CITY_NEIGHBORS(game_board.active_player().get_position());
    std::vector<Actions::Action*> full_list;
    for(int neighbor: Map::CITY_NEIGHBORS(game_board.active_player().get_position())){
        full_list.push_back(new Actions::Move(neighbor));
    }
    return full_list;
}

bool Actions::MoveConstructor::legal(Board::Board& game_board){
    // If player has expended 4 moves they can't move, otherwise they can.
    if(game_board.get_turn_action()<4){
        return true;
    } else {
        return false;
    }
}

// ========================

// ===== DIRECTFLIGHT =====
Actions::DirectFlight::DirectFlight(int city_idx): 
    citycard_index(city_idx){
    movetype = "DIRECTFLIGHT";
}

void Actions::DirectFlight::execute(Board::Board& new_board){
    Players::Player& active_player = new_board.active_player();

    // Set position to city represented by card
    active_player.set_position(Map::CITIES[citycard_index]);

    // Remove from player hand
    active_player.removeCard(citycard_index);

    new_board.get_turn_action()++;

    new_board.LastAction_DirectFlight=true;

    // Make sure disease is cleared from medic position, if there
    update_medic_position(new_board);
}

std::string Actions::DirectFlight::repr(){
    return movetype+" to "+Decks::CARD_NAME(citycard_index);
}

Actions::DirectFlightConstructor::DirectFlightConstructor(){}

std::string Actions::DirectFlightConstructor::get_movetype(){
    return movetype;
}

int Actions::DirectFlightConstructor::n_actions(Board::Board& game_board){
    if(legal(game_board)){
        Players::Player& active_player = game_board.active_player();

        int n_actions=0;
        for(int& card: active_player.hand){
            if(!isneighbor(card,active_player.get_position()) && card!=active_player.get_position()){
                n_actions++;
            }
        }
        return n_actions;
    } else { 
        return 0;
    }

}
Actions::Action* Actions::DirectFlightConstructor::random_action(Board::Board& game_board){
    Players::Player& active_player = game_board.active_player();
    std::vector<int> neighbors = Map::CITY_NEIGHBORS(active_player.get_position());
    int chosen_card = rand() % active_player.hand.size();

    while(isneighbor(active_player.hand[chosen_card],active_player.get_position()) || active_player.hand[chosen_card]==active_player.get_position()){
        // make sure it's not a neighbor or current position
        chosen_card = rand() % active_player.hand.size();
    }
    return new Actions::DirectFlight(active_player.hand[chosen_card]);
}

std::vector<Actions::Action*> Actions::DirectFlightConstructor::all_actions(Board::Board& game_board){
    std::vector<Actions::Action*> full_list = {};

    Players::Player& active_player = game_board.active_player();

    for(int card: active_player.hand){
        if(!isneighbor(card,active_player.get_position()) && card!=active_player.get_position()){
            full_list.push_back(new Actions::DirectFlight(card));
        }
    }
    return full_list;
}

bool Actions::DirectFlightConstructor::legal(Board::Board& game_board){
    // If it's during the player turn phase and they have city cards to discard, it's legal
    if(game_board.get_turn_action()<4){
        Players::Player& active_player = game_board.active_player();

        for(int& card: active_player.hand){
            if(!isneighbor(card,active_player.get_position()) 
                && card!=active_player.get_position()){
                return true;
            }
        }
    } 
    return false;
}
// ========================

// ===== CHARTERFLIGHT =====
Actions::CharterFlight::CharterFlight( int _target_city): 
    target_city(_target_city){
    movetype = "CHARTERFLIGHT";
}

void Actions::CharterFlight::execute(Board::Board& new_board){
    Players::Player& active_player = new_board.active_player();

    // Remove from player hand
    active_player.removeCard(active_player.get_position());

    // Set position to city represented by card
    active_player.set_position(target_city);

    new_board.get_turn_action()++;

    new_board.LastAction_CharterFlight=true;

    // Make sure disease is cleared from medic position, if there
    update_medic_position(new_board);
}

std::string Actions::CharterFlight::repr(){
    return movetype+" to "+Decks::CARD_NAME(target_city);
}

Actions::CharterFlightConstructor::CharterFlightConstructor(){}

std::string Actions::CharterFlightConstructor::get_movetype(){
    return movetype;
}

int Actions::CharterFlightConstructor::n_actions(Board::Board& game_board){
    if(legal(game_board)){
        Players::Player& active_player = game_board.active_player();
        return Map::CITIES.size() - 1 - Map::CITY_NEIGHBORS(active_player.get_position()).size();
    } else{
        return 0;
    }
    
}

Actions::Action* Actions::CharterFlightConstructor::random_action(Board::Board& game_board){
    Players::Player& active_player = game_board.active_player();
    std::vector<int> neighbors = Map::CITY_NEIGHBORS(active_player.get_position());

    int random_position = rand() % Map::CITIES.size(); // choose a random city to go to
    while(isneighbor(random_position,active_player.get_position()) || random_position==active_player.get_position()){
        random_position = rand() % Map::CITIES.size(); 
    }
    return new Actions::CharterFlight(random_position);
}

std::vector<Actions::Action*> Actions::CharterFlightConstructor::all_actions(Board::Board& game_board){
    std::vector<Actions::Action*> full_list;
    Players::Player& active_player = game_board.active_player();
    for(int n;n<Map::CITIES.size();n++){
        if(!isneighbor(n,active_player.get_position()) && n!=active_player.get_position()){
            full_list.push_back(new Actions::CharterFlight(n));
        }
    }
    return full_list;
}

bool Actions::CharterFlightConstructor::legal(Board::Board& game_board){
    Players::Player& active_player = game_board.active_player();
    // If it's the player turn phase and they have the card that matches the city they're in, return true
    for(int& c: active_player.hand){
        if(c==active_player.get_position()){
            return true;
        }
    }
    // But if no card matches their position return false
    return false;
}
// ========================

// ===== SHUTTLEFLIGHT =====
Actions::ShuttleFlight::ShuttleFlight( int _target_station): 
    target_station_city_idx(_target_station){
    movetype = "SHUTTLEFLIGHT";
}

void Actions::ShuttleFlight::execute(Board::Board& new_board){
    Players::Player& active_player = new_board.active_player();

    // Set position to city represented by target_station
    active_player.set_position(target_station_city_idx);

    new_board.get_turn_action()++;

    new_board.LastAction_ShuttleFlight=true;

    // Make sure disease is cleared from medic position, if there
    update_medic_position(new_board);
}

std::string Actions::ShuttleFlight::repr(){
    return movetype+" to "+Map::CITIES[target_station_city_idx].name;
}

Actions::ShuttleFlightConstructor::ShuttleFlightConstructor(){}

std::string Actions::ShuttleFlightConstructor::get_movetype(){
    return movetype;
}

int Actions::ShuttleFlightConstructor::n_actions(Board::Board& game_board){
    if(legal(game_board)){
        int n_actions=0;
        for(int st: game_board.get_stations()){
            if(st!=game_board.active_player().get_position()){
                n_actions++;
            }
        }
        return n_actions;
    } else {
        return 0;
    }
}

Actions::Action* Actions::ShuttleFlightConstructor::random_action(Board::Board& game_board){
    int position  = game_board.active_player().get_position();
    int random_position = rand() % game_board.get_stations().size();
    while(game_board.get_stations()[random_position]==position){
        // Keep randomizing until you find a station that isn't your present location
        random_position = rand() % game_board.get_stations().size(); 
    }
    return new Actions::ShuttleFlight(game_board.get_stations()[random_position]);
}

std::vector<Actions::Action*> Actions::ShuttleFlightConstructor::all_actions(Board::Board& game_board){
    std::vector<Actions::Action*> full_list;
    std::vector<int>& stations = game_board.get_stations();

    for(int& st: stations){
        if(st!=game_board.active_player().get_position()){
            full_list.push_back(new Actions::ShuttleFlight(st));
        } 
    }
    return full_list;
}

bool Actions::ShuttleFlightConstructor::legal(Board::Board& game_board){
    Players::Player& active_player = game_board.active_player();
    // if it's the player turn
    if(game_board.get_turn_action()<4){
        // and there's at least 2 stations
        if(game_board.get_stations().size()>1){
            for(int& st: game_board.get_stations()){
                // and the player is at a station
                if(st==active_player.get_position()){
                    for(int& other_st : game_board.get_stations()){
                        // and at least one of the other stations isn't their previous position...
                        if(other_st!=st){
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}
// ========================

// ===== OPERATIONSEXPERTFLIGHT =====
Actions::OperationsExpertFlight::OperationsExpertFlight( int _target_city,int _discard_card_idx): 
    target_city(_target_city),
    discard_card_card_idx(_discard_card_idx)
    {
    movetype = "OPERATIONSEXPERTFLIGHT";
}

void Actions::OperationsExpertFlight::execute(Board::Board& new_board){
    Players::Player& active_player = new_board.active_player();

    active_player.used_OperationsExpertFlight=true;

    // Remove discard_card from hand
    active_player.removeCard(discard_card_card_idx);

    // Set position to city represented by target_station
    active_player.set_position(target_city);

    new_board.get_turn_action()++;

    new_board.LastAction_OperationsExpertFlight=true;
}

std::string Actions::OperationsExpertFlight::repr(){
    return movetype+" to "+Decks::CARD_NAME(target_city)+" (discard "+Decks::CARD_NAME(discard_card_card_idx)+")";
}

Actions::OperationsExpertFlightConstructor::OperationsExpertFlightConstructor(){}

std::string Actions::OperationsExpertFlightConstructor::get_movetype(){
    return movetype;
}

int Actions::OperationsExpertFlightConstructor::n_actions(Board::Board& game_board){
    if(legal(game_board)){
        Players::Player& active_player = game_board.active_player();
        // Remove all neighbors and current position. multiply by hand size (# of discards)
        return (Map::CITIES.size() - 1 - Map::CITY_NEIGHBORS(active_player.get_position()).size()) * active_player.hand.size();
    } else {
        return 0;
    }
    
}

Actions::Action* Actions::OperationsExpertFlightConstructor::random_action(Board::Board& game_board){
    Players::Player& active_player = game_board.active_player();
    
    int position  = active_player.get_position();
    
    // Random destination
    int random_position = rand() % Map::CITIES.size();
    while(random_position==position || isneighbor(random_position,position)){
        random_position = rand() % Map::CITIES.size();
    }

    // Randomly discarded city card
    int random_discard = rand() % game_board.active_player().hand.size();

    return new Actions::OperationsExpertFlight(random_position,game_board.active_player().hand[random_discard]);
}

std::vector<Actions::Action*> Actions::OperationsExpertFlightConstructor::all_actions(Board::Board& game_board){
    std::vector<Actions::Action*> full_list;
    std::vector<int> hand = game_board.active_player().hand;
    Players::Player& active_player = game_board.active_player();

    for(int n;n<Map::CITIES.size();n++){
        if(!isneighbor(n,active_player.get_position()) && n!=active_player.get_position()){
            for(int c=0; c<hand.size();c++){
                full_list.push_back(new Actions::OperationsExpertFlight(n,hand[c]));
            } 
        }
    }
    return full_list;
}

bool Actions::OperationsExpertFlightConstructor::legal(Board::Board& game_board){
    Players::Player& active_player = game_board.active_player();
    // If active player is the operations expert
    if(active_player.role.operationsexpert){
        // and it's still players turn to do stuff
        if(game_board.get_turn_action()<4){
            for(int& st: game_board.get_stations()){
                // And they're at the position of a station
                if(st==active_player.get_position()){
                    // And they have at least one card to discard
                    if(active_player.hand.size()>0){
                        return true;
                    }
                }
            }
        }
    }
    return false;
}
// ========================

// ===== BUILD =====
Actions::Build::Build( int _place_station,int _remove_station): 
    remove_station(_remove_station),
    place_station(_place_station)
    {
    movetype = "BUILD";

}

void Actions::Build::execute(Board::Board& new_board){
    Players::Player& active_player = new_board.active_player();

    // If the remove_station argument>=0 and so representing a station to remove
    if(remove_station>=0){
        // Should break catastrophically when remove_station isn't defined properly
        // (Should be city index of station to remove)
        new_board.RemoveStation(remove_station);
    }

    // Add it to the vector of research station Map::City elements
    new_board.AddStation(place_station);

    //Check whether active player is Operations Expert. If not, discard necessary card.
    if(!active_player.role.operationsexpert){
        active_player.removeCard(place_station);
    }

    new_board.get_turn_action()++;

    new_board.LastAction_Build=true;
}

std::string Actions::Build::repr(){
    std::string str_out = movetype+" at "+Map::CITY_NAME(place_station);
    if(remove_station>=0){
        return str_out + " (remove the one at " + Map::CITY_NAME(remove_station) + ")";
    } else {
        return str_out;
    }
}

Actions::BuildConstructor::BuildConstructor(){}

std::string Actions::BuildConstructor::get_movetype(){
    return movetype;
}

int Actions::BuildConstructor::n_actions(Board::Board& game_board){
    if(legal(game_board)){
        if(game_board.get_stations().size()>=6){
            return game_board.get_stations().size(); // Can remove any existing stations. (In a vanilla game this can't be more than 6)
        } else {
            return 1; // otherwise there's only one thing to do
        }
    } else {
        return 0;
    }
    
}

Actions::Action* Actions::BuildConstructor::random_action(Board::Board& game_board){
    Players::Player& active_player = game_board.active_player();

    if(game_board.get_stations().size()>=6){
        return new Actions::Build(active_player.get_position(),rand() % game_board.get_stations().size());
    } else {
        return new Actions::Build(active_player.get_position(),-1);
    }
}

std::vector<Actions::Action*> Actions::BuildConstructor::all_actions(Board::Board& game_board){
    std::vector<Actions::Action*> full_list;
    std::vector<int>& stations = game_board.get_stations();

    Players::Player& active_player = game_board.active_player();

    if(stations.size()>=6){
        // Should never be >6 but just in case...
        for(int st : game_board.get_stations()){
            // If this station isn't at your current location (shouldn't be possible when guarded by legal())
            if(st!=active_player.get_position()){
                full_list.push_back(new Actions::Build(active_player.get_position(),st));
            } 
        }
    } else {
        full_list.push_back(new Actions::Build(active_player.get_position()));
    }
    return full_list;
}

bool Actions::BuildConstructor::legal(Board::Board& game_board){
    Players::Player& active_player = game_board.active_player();
    if(game_board.get_turn_action()<4){
        if(active_player.role.operationsexpert){
            bool already_station = false;
            for(int st: game_board.get_stations()){
                if(st==active_player.get_position()){
                    already_station=true;
                    break;
                }
            }
            if(!already_station){
                return true;
            }
        } else{
            for(int& c : active_player.hand){
                // Then if they have a card representing the city they're in...
                if(c==active_player.get_position()){
                    bool already_station = false;
                    // If no station is already on this city..
                    for(int st: game_board.get_stations()){
                        if(st==active_player.get_position()){
                            already_station=true;
                            break;
                        }
                    }
                    // then active_player can build a station
                    if(!already_station){
                        return true;
                    }
                }
            }
        }
    }
    return false;
}
// ========================

// ===== TREAT =====
Actions::Treat::Treat( int _color): 
    color(_color){
    movetype = "TREAT";
}

void Actions::Treat::execute(Board::Board& new_board){
    Players::Player& active_player = new_board.active_player();
    
    if(active_player.role.medic || new_board.is_cured(color)){
        n_treated = new_board.get_disease_count()[color][active_player.get_position()];
        new_board.get_disease_count()[color][active_player.get_position()]=0;
    } else {
        n_treated=1;
        new_board.get_disease_count()[color][active_player.get_position()]--;
    }

    new_board.get_color_count()[color]-=n_treated;
    
    new_board.get_turn_action()++;
    // Check for eradication
    // If(this color is cured && there are now 0 cubes of this disease)
    if(new_board.is_cured(color) && new_board.disease_sum(color)==0){
        new_board.Eradicate(color);
    }

    new_board.LastAction_Treat=true;
}

std::string Actions::Treat::repr(){
    return movetype+" " + std::to_string(n_treated) +" "+ Map::COLORS[color];
}

Actions::TreatConstructor::TreatConstructor(){}

std::string Actions::TreatConstructor::get_movetype(){
    return movetype;
}

int Actions::TreatConstructor::n_actions(Board::Board& game_board){
    if(legal(game_board)){
        int n_actions = 0;
        Players::Player& active_player = game_board.active_player();
        if(game_board.get_disease_count()[Map::BLUE][active_player.get_position()]>0){
            n_actions++;
        }
        if(game_board.get_disease_count()[Map::YELLOW][active_player.get_position()]>0){
            n_actions++;
        }
        if(game_board.get_disease_count()[Map::BLACK][active_player.get_position()]>0){
            n_actions++;
        }
        if(game_board.get_disease_count()[Map::RED][active_player.get_position()]>0){
            n_actions++;
        }
        return n_actions; // 1 for each color of disease on the position of the current city
    } else {
        return 0;
    }
     
}

Actions::Action* Actions::TreatConstructor::random_action(Board::Board& game_board){
    std::vector<int> nonzero_colors = {};
    Players::Player& active_player = game_board.active_player();
    if(game_board.get_disease_count()[Map::BLUE][active_player.get_position()]>0){
        nonzero_colors.push_back(Map::BLUE);
    }
    if(game_board.get_disease_count()[Map::YELLOW][active_player.get_position()]>0){
        nonzero_colors.push_back(Map::YELLOW);
    }
    if(game_board.get_disease_count()[Map::BLACK][active_player.get_position()]>0){
        nonzero_colors.push_back(Map::BLACK);
    }
    if(game_board.get_disease_count()[Map::RED][active_player.get_position()]>0){
        nonzero_colors.push_back(Map::RED);
    }
    return new Actions::Treat(nonzero_colors[rand() % nonzero_colors.size()]);
}

std::vector<Actions::Action*> Actions::TreatConstructor::all_actions(Board::Board& game_board){
    std::vector<Actions::Action*> full_list;
    Players::Player& active_player = game_board.active_player();
    if(game_board.get_disease_count()[Map::BLUE][active_player.get_position()]>0){
        full_list.push_back(new Actions::Treat(Map::BLUE));
    }
    // If I do it this way, then the fast-compiled version won't decide to skip a loop?
    if(game_board.get_disease_count()[Map::YELLOW][active_player.get_position()]>0){
        full_list.push_back(new Actions::Treat(Map::YELLOW));
    }
    // If I do it this way, then the fast-compiled version won't decide to skip a loop?
    if(game_board.get_disease_count()[Map::BLACK][active_player.get_position()]>0){
        full_list.push_back(new Actions::Treat(Map::BLACK));
    }
    // If I do it this way, then the fast-compiled version won't decide to skip a loop?
    if(game_board.get_disease_count()[Map::RED][active_player.get_position()]>0){
        full_list.push_back(new Actions::Treat(Map::RED));
    }
    return full_list;
}

bool Actions::TreatConstructor::legal(Board::Board& game_board){
    // If it's a players turn...
    if(game_board.get_turn_action()<4){
        Players::Player& active_player = game_board.active_player();
        // And there's a color that has >0 cubes at the present city, it's legal
        
        // If I do it this way, then the fast-compiled version won't decide to skip a loop?
        if(game_board.get_disease_count()[Map::BLUE][active_player.get_position()]>0){
            return true;
        }
        // If I do it this way, then the fast-compiled version won't decide to skip a loop?
        if(game_board.get_disease_count()[Map::YELLOW][active_player.get_position()]>0){
            return true;
        }
        // If I do it this way, then the fast-compiled version won't decide to skip a loop?
        if(game_board.get_disease_count()[Map::BLACK][active_player.get_position()]>0){
            return true;
        }
        // If I do it this way, then the fast-compiled version won't decide to skip a loop?
        if(game_board.get_disease_count()[Map::RED][active_player.get_position()]>0){
            return true;
        }
    }
    return false;
}
// ========================

// ===== CURE =====
Actions::Cure::Cure() 
    {
    movetype = "CURE";
}

void Actions::Cure::execute(Board::Board& new_board){
    Players::Player& active_player = new_board.active_player();

    for(int st: new_board.get_stations()){
        if(st==active_player.get_position()){
            std::array<int,4> color_count = active_player.get_color_count();

            if(color_count[Map::BLUE]>=active_player.role.required_cure_cards && !new_board.is_cured(Map::BLUE)){
                color_cured = Map::BLUE;
                active_player.removeCureCardColor(Map::BLUE);
                new_board.Cure(Map::BLUE);
                new_board.get_turn_action()++;
                // Update eradicated status if there is no disease of this color on the board
                if(new_board.disease_sum(Map::BLUE)==0){
                    new_board.Eradicate(Map::BLUE);
                }
                new_board.LastAction_Cure=true;
                if(new_board.is_cured(Map::BLUE) && new_board.is_cured(Map::YELLOW) && new_board.is_cured(Map::BLACK) && new_board.is_cured(Map::RED)){
                    new_board.has_won()=true;
                }
                // Make sure disease is cleared from medic position, if there
                update_medic_position(new_board);
                return;
            }
            if(color_count[Map::YELLOW]>=active_player.role.required_cure_cards && !new_board.is_cured(Map::YELLOW)){
                color_cured = Map::YELLOW;
                active_player.removeCureCardColor(Map::YELLOW);
                new_board.Cure(Map::YELLOW);
                new_board.get_turn_action()++;
                // Update eradicated status if there is no disease of this color on the board
                if(new_board.disease_sum(Map::YELLOW)==0){
                    new_board.Eradicate(Map::YELLOW);
                }
                new_board.LastAction_Cure=true;
                if(new_board.is_cured(Map::BLUE) && new_board.is_cured(Map::YELLOW) && new_board.is_cured(Map::BLACK) && new_board.is_cured(Map::RED)){
                    new_board.has_won()=true;
                }
                // Make sure disease is cleared from medic position, if there
                update_medic_position(new_board);
                return;
            }
            if(color_count[Map::BLACK]>=active_player.role.required_cure_cards && !new_board.is_cured(Map::BLACK)){
                color_cured = Map::BLACK;
                active_player.removeCureCardColor(Map::BLACK);
                new_board.Cure(Map::BLACK);
                new_board.get_turn_action()++;
                // Update eradicated status if there is no disease of this color on the board
                if(new_board.disease_sum(Map::BLACK)==0){
                    new_board.Eradicate(Map::BLACK);
                }
                new_board.LastAction_Cure=true;
                if(new_board.is_cured(Map::BLUE) && new_board.is_cured(Map::YELLOW) && new_board.is_cured(Map::BLACK) && new_board.is_cured(Map::RED)){
                    new_board.has_won()=true;
                }
                // Make sure disease is cleared from medic position, if there
                update_medic_position(new_board);
                return;
            }
            if(color_count[Map::RED]>=active_player.role.required_cure_cards && !new_board.is_cured(Map::RED)){
                color_cured = Map::RED;
                active_player.removeCureCardColor(Map::RED);
                new_board.Cure(Map::RED);
                new_board.get_turn_action()++;
                // Update eradicated status if there is no disease of this color on the board
                if(new_board.disease_sum(Map::RED)==0){
                    new_board.Eradicate(Map::RED);
                }
                new_board.LastAction_Cure=true;
                if(new_board.is_cured(Map::BLUE) && new_board.is_cured(Map::YELLOW) && new_board.is_cured(Map::BLACK) && new_board.is_cured(Map::RED)){
                    new_board.has_won()=true;
                }
                // Make sure disease is cleared from medic position, if there
                update_medic_position(new_board);
                return;
            }
        }
    }
    new_board.broken()=true;
    new_board.broken_reasons().push_back("[Cure::execute()] CURE execute() was called but there don't seem to be enough of any card to cure, or they weren't at a research station...");
}

std::string Actions::Cure::repr(){
    return movetype + " " + Map::COLORS[color_cured];
}

Actions::CureConstructor::CureConstructor(){}

std::string Actions::CureConstructor::get_movetype(){
    return movetype;
}

int Actions::CureConstructor::n_actions(Board::Board& game_board){
    if(legal(game_board)){
        return 1; //  If this is available, there's only 1 curable disease by the active player.
    } else {
        return 0;
    }
}

Actions::Action* Actions::CureConstructor::random_action(Board::Board& game_board){
    return new Actions::Cure();// if it's legal, then there's only one choice.
}

std::vector<Actions::Action*> Actions::CureConstructor::all_actions(Board::Board& game_board){
    std::vector<Actions::Action*> full_list;
    full_list.push_back(new Actions::Cure());
    return full_list; // Like above; if it's legal then there's only one action: list of size 1
}

bool Actions::CureConstructor::legal(Board::Board& game_board){
    // If it's a players turn...
    if(game_board.get_turn_action()<4){
        Players::Player& active_player = game_board.active_player();

        std::array<int,4> color_count = active_player.get_color_count();

        // and they have the required cards for an uncured color        
        if((color_count[Map::BLUE]>=active_player.role.required_cure_cards && !game_board.is_cured(Map::BLUE)) ||
            (color_count[Map::YELLOW]>=active_player.role.required_cure_cards && !game_board.is_cured(Map::YELLOW)) ||
            (color_count[Map::BLACK]>=active_player.role.required_cure_cards && !game_board.is_cured(Map::BLACK)) ||
            (color_count[Map::RED]>=active_player.role.required_cure_cards && !game_board.is_cured(Map::RED))){
            for(int st: game_board.get_stations()){
                // And they're at a research station...
                if(st==active_player.get_position()){
                    // Then they can cure
                    return true;
                }
            }
        }
    }
    return false;
}
// ========================

// ===== GIVE =====
Actions::Give::Give(Players::Player _other_player, int _card_to_give_cityidx):
    other_player(_other_player),
    card_to_give_city_idx(_card_to_give_cityidx)
    {
    movetype = "GIVE";
}

void Actions::Give::execute(Board::Board& new_board){
    Players::Player& active_player = new_board.active_player();

    active_player.removeCard(card_to_give_city_idx);

    for(Players::Player& p: new_board.get_players()){
        if(p.role==other_player.role){
            p.UpdateHand(card_to_give_city_idx);
        }
    }
    new_board.get_turn_action()++;

    new_board.LastAction_Give=true;
}

std::string Actions::Give::repr(){
    return movetype + " " + Decks::CARD_NAME(card_to_give_city_idx) + " to " + other_player.role.name;
}

Actions::GiveConstructor::GiveConstructor(){}

std::string Actions::GiveConstructor::get_movetype(){
    return movetype;
}

int Actions::GiveConstructor::n_actions(Board::Board& game_board){
    if(legal(game_board)){
        Players::Player& active_player = game_board.active_player();
        // Can only ever be # of other players on this city, otherwise whole hand * # of other players on this city
        int n_other_players_here = 0; 
        // For every player, check whether they're in the same position as active_player but aren't active_player
        for(Players::Player& _other_player: game_board.get_players()){
            // If they're in the same city...
            if(_other_player.get_position()==active_player.get_position() && active_player.role!=_other_player.role){
                n_other_players_here++;
            }
        }
        if(active_player.role.researcher){
            return active_player.hand.size()*n_other_players_here;
        } else {
            return n_other_players_here;
        }
    } else {
        return 0;
    }  
}

Actions::Action* Actions::GiveConstructor::random_action(Board::Board& game_board){
    Players::Player& active_player = game_board.active_player();
    int card_to_give = -1;

    // First randomize the card that active_player will give based on their role
    if(active_player.role.researcher){
        card_to_give = active_player.hand[rand() % active_player.hand.size()];
    } else {
        card_to_give = active_player.get_position();
    }

    // Check how many other players are here.
    int n_other_players_here = 0; // initialize at -1 since we'll iterate through all players and see active_player at current city
    for(Players::Player& _other_player: game_board.get_players()){
        // If they're in the same city...
        if(_other_player.get_position()==active_player.get_position() && active_player.role!=_other_player.role){
            n_other_players_here++;
        }
    }

    // Randomized value to choose which other player at this city we'll give to
    int which_player = rand() % n_other_players_here; // 0 ... (# of other players in city - 1)
    // Incremented value to track which of the other players on this city we're considering
    int track=0;
    for(Players::Player _other_player: game_board.get_players()){
        // If the other player is here and isn't the active_player...
        if(_other_player.get_position()==active_player.get_position() && active_player.role!=_other_player.role){
            // If track== the random value generated, return an action!
            if(track==which_player){
                return new Actions::Give(_other_player,card_to_give);
            }
            // Otherwise increment up track; it's a different player we want at this city.
            track++;
        }
    }
    game_board.broken()=true;
    game_board.broken_reasons().push_back("[GiveConstructor::random_action()] GIVE failed to return a random action!");
    return nullptr;
}

std::vector<Actions::Action*> Actions::GiveConstructor::all_actions(Board::Board& game_board){
    std::vector<Actions::Action*> full_list;
    Players::Player& active_player = game_board.active_player();
    std::vector<int> cards_to_give = {};

    if(active_player.role.researcher){
        cards_to_give = active_player.hand;
    } else {
        cards_to_give = {active_player.get_position()};
    }

    for(int p=0;p<game_board.get_players().size();p++){
        // for each other player 
        Players::Player& _other_player = game_board.get_players()[p];
        if(_other_player.get_position()==active_player.get_position() && active_player.role!=_other_player.role){
            // if they're at this location
            for(int c=0;c<cards_to_give.size();c++){
                // then we *could* give them a card
                full_list.push_back(new Actions::Give(_other_player,cards_to_give[c]));
            }
        }
    }

    return full_list;
}

bool Actions::GiveConstructor::legal(Board::Board& game_board){
    // If its the player turn
    if(game_board.get_turn_action()<4){
        Players::Player& active_player = game_board.active_player();
        for(Players::Player& p: game_board.get_players()){
            // And there's another player at the active players position
            if(p.get_position()==active_player.get_position() && active_player.role!=p.role){
                if(active_player.hand.size()>0){
                    // And the active player has cards
                    if(active_player.role.researcher){
                        return true;
                    } else {
                        for(int& card: active_player.hand){
                            if(card==active_player.get_position()){
                                return true;
                            }
                        }
                    }
                }
            }
        }
    }
    return false;
}
// ========================

// ===== TAKE =====
Actions::Take::Take( Players::Player _other_player, int _card_to_take):
    other_player(_other_player),
    card_to_take_city_idx(_card_to_take)
    {
    movetype = "TAKE";
}

void Actions::Take::execute(Board::Board& new_board){
    Players::Player& active_player = new_board.active_player();

    for(Players::Player& p: new_board.get_players()){
        if(p.role==other_player.role){
            p.removeCard(card_to_take_city_idx);
        }
    }

    active_player.UpdateHand(card_to_take_city_idx);
    new_board.get_turn_action()++;

    new_board.LastAction_Take=true;
}

std::string Actions::Take::repr(){
    return movetype + " " + Decks::CARD_NAME(card_to_take_city_idx) + " from " + other_player.role.name;
}

Actions::TakeConstructor::TakeConstructor(){}

std::string Actions::TakeConstructor::get_movetype(){
    return movetype;
}

int Actions::TakeConstructor::n_actions(Board::Board& game_board){
    if(legal(game_board)){
        Players::Player& active_player = game_board.active_player();
    
        // Have to check each same-city player and add based on researcher status and active_player hand fullness
        int n_actions = 0; 
        // For every player, check whether they're in the same position as active_player but aren't active_player
        for(Players::Player& _other_player : game_board.get_players()){
            // If they're in the same city...
            if(_other_player.get_position()==active_player.get_position() && active_player.role!=_other_player.role){
                // and it's the researcher...
                if(_other_player.role.researcher){
                    // then we could take any of their cards
                    n_actions+=_other_player.hand.size();
                } else {
                    // We just add 1 if they have a card we could take (the city we're in)
                    for(int& c: _other_player.hand){
                        if(c==active_player.get_position()){
                            n_actions+=1;
                        }
                    }
                }
            }
        }
        return n_actions;
    } else {
        return 0;
    }
}

Actions::Action* Actions::TakeConstructor::random_action(Board::Board& game_board){

    // I tried thinking of a way to do this without calling all actions
    // But that seemed at least as hard as just doing this
    // This gives uniformity over all cards to take
    std::vector<Actions::Action*> action_list = all_actions(game_board);

    int chosen_action_index = rand() % action_list.size();

    // Have to address that this is a potential memory leak though
    for(int ind=0;ind<action_list.size();ind++){
        // delete all actions that aren't the one to return
        if(ind!=chosen_action_index){
            delete action_list[ind];
        }
    }
    return action_list[chosen_action_index];
}

std::vector<Actions::Action*> Actions::TakeConstructor::all_actions(Board::Board& game_board){
    std::vector<Actions::Action*> full_list;
    Players::Player& active_player = game_board.active_player();
    std::vector<int> cards_to_take = {};

    for(Players::Player _other_player : game_board.get_players()){
        // If the other player is at active_players position
        if(_other_player.get_position()==active_player.get_position() && active_player.role!=_other_player.role){
            // Then check through each card...
            for(int card: _other_player.hand){
                // And if they're a researcher or the card index is the same as active_players position
                if(card==active_player.get_position() || _other_player.role.researcher){
                    // Then add it to the list
                    full_list.push_back(new Actions::Take(_other_player,card));
                }
            }
        }
    }
    return full_list;
}

bool Actions::TakeConstructor::legal(Board::Board& game_board){
    if(game_board.get_turn_action()<4){
        Players::Player& active_player = game_board.active_player();
        for(Players::Player& _other_player: game_board.get_players()){

            if(_other_player.get_position()==active_player.get_position() && active_player.role!=_other_player.role){
                if(_other_player.role.researcher){
                    if(_other_player.hand.size()>0){
                        return true;
                    }
                } else {
                    for(int& card: _other_player.hand){
                        if(card==active_player.get_position()){
                            // Then you're in the same city as another player and they have the card of this city
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}
// ========================

// ===== AIRLIFT =====
Actions::Airlift::Airlift( Players::Player _using_player, Players::Player _target_player,int _target_city):
    using_player(_using_player),
    target_player(_target_player),
    target_city(_target_city) // doesn't have to be referenced
    {
    movetype = "AIRLIFT";
}

void Actions::Airlift::execute(Board::Board& new_board){
    for(Players::Player& p: new_board.get_players()){
        // If they're the target_player in new_board, set their position
        if(p.role==target_player.role){
            p.set_position(Map::CITIES[target_city]);
        }
        // If they're the using player, remove the card from their hand
        if(p.role==using_player.role){
            p.removeCard(50);
        }
    }

    new_board.LastAction_Airlift=true;
}

std::string Actions::Airlift::repr(){
    return movetype +" " +using_player.role.name +" airlifts " + target_player.role.name + " to " + Map::CITIES[target_city].name;
}

Actions::AirliftConstructor::AirliftConstructor(){}

std::string Actions::AirliftConstructor::get_movetype(){
    return movetype;
}

int Actions::AirliftConstructor::n_actions(Board::Board& game_board){
    if(legal(game_board)){
        // Every player to every location but their current one
        return (Map::CITIES.size()-1)*game_board.get_players().size();
    } else {
        return 0;
    }
}

Actions::Action* Actions::AirliftConstructor::random_action(Board::Board& game_board){
    // copy a player that will be targeted
    Players::Player _target_player = game_board.get_players()[rand() % game_board.get_players().size()];
    // Use dumb loop to find the player that's using this action.
    for(Players::Player _using_player: game_board.get_players()){
        // Look for Airlift in the player's event_cards
        for(int e: _using_player.event_cards){
            if(Decks::CARD_NAME(e)=="Airlift" && e==50){
                int random_city = rand() % Map::CITIES.size();
                while(random_city==_target_player.get_position() || isneighbor(random_city,_target_player.get_position())){
                    random_city = rand() % Map::CITIES.size();
                }
                return new Actions::Airlift(_using_player,_target_player,random_city);
            }
        }
    }
    game_board.broken()=true;
    game_board.broken_reasons().push_back("[AirliftConstructor::random_action()] AIRLIFT random_action() called and didn't return an action");
    return nullptr;
}

std::vector<Actions::Action*> Actions::AirliftConstructor::all_actions(Board::Board& game_board){
    std::vector<Actions::Action*> full_list;
    for(Players::Player _using_player : game_board.get_players()){
        // Look for Airlift in the player's event_cards
        for(int e: _using_player.event_cards){
            // If we found the airlift-holder...
            if(Decks::CARD_NAME(e)=="Airlift" && e==50){
                // Then for every possible target player...
                for(Players::Player _target_player : game_board.get_players()){
                    for(int city=0;city<Map::CITIES.size();city++){
                        // for every city that's not a neighbor of the target player or their current position
                        if(!isneighbor(city,_target_player.get_position()) && city!=_target_player.get_position()){
                            // add an action.
                            full_list.push_back(new Actions::Airlift(_using_player,_target_player,city));
                        }
                    }
                    
                }
            }
        }
    }
    return full_list;
}

bool Actions::AirliftConstructor::legal(Board::Board& game_board){
    // The legality guard is always true when the card is held by any player
    for(Players::Player& pl : game_board.get_players()){
        for(int& e : pl.event_cards){
            if(Decks::CARD_NAME(e)=="Airlift" && e==50){
                return true;
            }
        }
    }
    return false;
}
// ========================

// ===== GOVERNMENTGRANT =====
Actions::GovernmentGrant::GovernmentGrant( Players::Player _using_player, int _target_city,int _remove_station):
    using_player(_using_player),
    target_city(_target_city),
    remove_station(_remove_station)
    {
    movetype = "GOVERNMENTGRANT";
}

void Actions::GovernmentGrant::execute(Board::Board& new_board){
    for(Players::Player& p: new_board.get_players()){
        if(p.role==using_player.role){
            p.removeCard(49);
        }
    }
    
    // Add it to the vector of research station Map::City elements
    new_board.AddStation(target_city);

    if(remove_station>=0){
        // Should break catastrophically when remove_station isn't defined properly
        // (Should be city index of station to be removed)
        new_board.RemoveStation(remove_station);
    }

    new_board.LastAction_GovernmentGrant=true;
}

std::string Actions::GovernmentGrant::repr(){
    std::string out_str = movetype + " " + using_player.role.name + " put a research station at " + Map::CITIES[target_city].name;
    if(remove_station>=0){
        out_str+=" (removed " + Map::CITIES[remove_station].name + ")";
    }
    return out_str;
}

Actions::GovernmentGrantConstructor::GovernmentGrantConstructor(){}

std::string Actions::GovernmentGrantConstructor::get_movetype(){
    return movetype;
}

int Actions::GovernmentGrantConstructor::n_actions(Board::Board& game_board){
    if(legal(game_board)){
        if(game_board.get_stations().size()>=6){
            return game_board.get_stations().size()*(Map::CITIES.size()-game_board.get_stations().size()); // can place a new station at any of 48-(# stations) locations, and remove any of (# current stations)
        } else
            return Map::CITIES.size() - game_board.get_stations().size(); // can place anywhere there isn't a station currently
    } else {
        return 0;
    }
}

Actions::Action* Actions::GovernmentGrantConstructor::random_action(Board::Board& game_board){
    // I feel like I shouldn't have written anything while drinking
    int target_city = Map::CITIES[rand() % Map::CITIES.size()].index;
    bool already_station = true;
    // Loop until 4 sure target_city isn't already occupied by a research station
    while(already_station){
        already_station = false; 
        for(int st: game_board.get_stations()){
            if(st==target_city){
                target_city = Map::CITIES[rand() % Map::CITIES.size()].index;
                already_station=true;
                break;
            }
        }
    }

    // at this point target_city is guaranteed to not have an existing station.

    for(Players::Player this_player : game_board.get_players()){
        for(int e : this_player.event_cards){
            if(e==49 && Decks::CARD_NAME(e)=="Government Grant"){
                if(game_board.get_stations().size()>=6){
                    // remove a station at random using its city index
                    return new Actions::GovernmentGrant(this_player,target_city,game_board.get_stations()[rand() % game_board.get_stations().size()]);
                }else{
                    return new Actions::GovernmentGrant(this_player,target_city);
                }  
            }
        }
    }
    game_board.broken()=true;
    game_board.broken_reasons().push_back("[GovernmentGrantConstructor::random_action()] GOVERNMENTGRANT random_action() called but didn't return an action");
    return nullptr;
}

std::vector<Actions::Action*> Actions::GovernmentGrantConstructor::all_actions(Board::Board& game_board){
    // This monstrosity returns a vector of actions to insert a new research station when:
    //      - It's not in the same position as any existing station

    std::vector<Actions::Action*> full_list;
    for(Players::Player _using_player: game_board.get_players()){
        // Find the using player by looking through event cards
        for(int e : _using_player.event_cards){
            if(e==49 && Decks::CARD_NAME(e)=="Government Grant"){ // Index of government grant
                if(game_board.get_stations().size()>=6){
                    for(int city=0;city<Map::CITIES.size();city++){
                        bool already_exists = false;
                        for(int st: game_board.get_stations()){
                            if(city==st){
                                already_exists=true;
                                break;
                            }   
                        }
                        if(!already_exists){
                            for(int st=0;st<game_board.get_stations().size();st++){
                                full_list.push_back(new Actions::GovernmentGrant(_using_player,city,game_board.get_stations()[st]));
                            }  
                        }
                    }
                } else {
                    for(int c=0;c<Map::CITIES.size();c++){
                        bool already_exists=false;
                        for(int st: game_board.get_stations()){
                            if(c==st){
                                already_exists=true;
                                break;
                            } 
                        }
                        if(!already_exists){
                            full_list.push_back(new Actions::GovernmentGrant(_using_player,c));
                        }
                    }
                }
            }
        }
    }
    return full_list;
}

bool Actions::GovernmentGrantConstructor::legal(Board::Board& game_board){
    // make it legal only at beginning of player turn (if it's good to use during the player turn, then might as well use it at beginning)
    if(game_board.get_turn_action()==0){
        for(Players::Player& pl : game_board.get_players()){
            for(int& e : pl.event_cards){
                if(Decks::CARD_NAME(e)=="Government Grant" && e==49){
                    return true;
                }
            }
        }
    }
    
    return false;
}
// ========================

// ===== QUIETNIGHT =====
Actions::QuietNight::QuietNight( Players::Player _using_player):
    using_player(_using_player)
    {
    movetype = "QUIETNIGHT";
}

void Actions::QuietNight::execute(Board::Board& new_board){
    for(Players::Player& p: new_board.get_players()){
        if(p.role==using_player.role){
            p.removeCard(48);
        }
    }
    new_board.quiet_night_status() = true;

    new_board.LastAction_QuietNight=true;
}

std::string Actions::QuietNight::repr(){
    return movetype + " used by " + using_player.role.name;
}

Actions::QuietNightConstructor::QuietNightConstructor(){}

std::string Actions::QuietNightConstructor::get_movetype(){
    return movetype;
}

int Actions::QuietNightConstructor::n_actions(Board::Board& game_board){
    if(legal(game_board)){
        return 1;
    } else {
        return 0;
    }
}

Actions::Action* Actions::QuietNightConstructor::random_action(Board::Board& game_board){
    for(Players::Player this_player: game_board.get_players()){
        for(int e : this_player.event_cards){
            if(e==48 && Decks::CARD_NAME(e)=="Quiet Night"){
                return new Actions::QuietNight(this_player);
            }
        }
    }
    game_board.broken()=true;
    game_board.broken_reasons().push_back("[QuietNightConstructor::random_action()] QUIETNIGHT random_action() called and didn't return an action");
    return nullptr;
}

std::vector<Actions::Action*> Actions::QuietNightConstructor::all_actions(Board::Board& game_board){
    std::vector<Actions::Action*> full_list;

    // Find the player that owns the card and return the action with them as the user
    for(int p=0;p<game_board.get_players().size();p++){
        Players::Player this_player = game_board.get_players()[p];
        for(int e : this_player.event_cards){
            if(e==48 && Decks::CARD_NAME(e)=="Quiet Night"){
                full_list.push_back(new Actions::QuietNight(this_player));
                return full_list;
            }
        }
    }
    game_board.broken()=true;
    game_board.broken_reasons().push_back("[QuietNightConstructor::all_actions()] QUIETNIGHT all_actions() called and didn't return any actions");
    return {nullptr};
}

bool Actions::QuietNightConstructor::legal(Board::Board& game_board){
    // Only allow this at the beginning of player turn: really doesn't make a difference when, if ever, it's used during player turn so only have to consider once
    if(game_board.get_turn_action()==0){
        for(Players::Player& pl : game_board.get_players()){
            for(int& e : pl.event_cards){
                if(Decks::CARD_NAME(e)=="Quiet Night" && e==48){
                    return true;
                }
            }
        }
    }
    return false;
}
// ========================

// ===== DONOTHING =====
Actions::DoNothing::DoNothing()
    {
    movetype = "DONOTHING";
}

void Actions::DoNothing::execute(Board::Board& game_board){
    // If used as an "action" during a player turn, increment the counter
    if(game_board.get_turn_action()<4){
        game_board.get_turn_action()++;
    }
    // Otherwise this is being called when perhaps using an Event card outside of the turn, and shouldn't change the game state at all.
    game_board.LastAction_DoNothing=true;
}

std::string Actions::DoNothing::repr(){
    return movetype;
}

Actions::DoNothingConstructor::DoNothingConstructor(){}

std::string Actions::DoNothingConstructor::get_movetype(){
    return movetype;
}

int Actions::DoNothingConstructor::n_actions(Board::Board& game_board){
    if(legal(game_board)){
        return 1; // exactly 1 way to do jack
    } else {
        return 0;
    }
}

Actions::Action* Actions::DoNothingConstructor::random_action(Board::Board& game_board){
    return new Actions::DoNothing();
}

std::vector<Actions::Action*> Actions::DoNothingConstructor::all_actions(Board::Board& game_board){
    return {new Actions::DoNothing()};
}

bool Actions::DoNothingConstructor::legal(Board::Board& game_board){
    // Can only do nothing if it's beginning of player turn or player has "done something"
    if(game_board.get_turn_action()==0){
        return true;
    }
    return false;
}
// ========================

// ===== FORCED DISCARD =====
// Can return an action to discard a city card, or use an event card!
Actions::ForcedDiscardAction::ForcedDiscardAction( Players::Player _player_to_discard,int _discard_card_index):
    discard_card_index(_discard_card_index),
    player_to_discard(_player_to_discard){
        movetype = "FORCEDDISCARD";
}

std::string Actions::ForcedDiscardAction::repr(){
    return movetype+" " + player_to_discard.role.name + " discarded " + Decks::CARD_NAME(discard_card_index);
}

void Actions::ForcedDiscardAction::execute(Board::Board& new_board){
    for(Players::Player& p : new_board.get_players()){
        if(p.role==player_to_discard.role){
            p.removeCard(discard_card_index);
        }
    }
    new_board.LastAction_ForcedDiscard=true;
}

Actions::ForcedDiscardConstructor::ForcedDiscardConstructor(){};

std::string Actions::ForcedDiscardConstructor::get_movetype(){
    return movetype;
}

int Actions::ForcedDiscardConstructor::n_actions(Board::Board& game_board){
    // Find the first player whose hand is full and build discard actions
    // Unlike other actions, we DON'T care about legality guards here, since it won't ever (and shouldn't) be considered for random selection by an agent.
    for(Players::Player& p: game_board.get_players()){
        if(p.hand_full()){
            int total_actions = p.hand.size(); // initialize with # of city cards
            for(int& e: p.event_cards){
                switch(e){
                    case 48:
                        total_actions+=Actions::QuietNightConstructor().n_actions(game_board);
                        break;
                    case 49:
                        total_actions+=Actions::GovernmentGrantConstructor().n_actions(game_board);
                        break;
                    case 50:
                        total_actions+=Actions::AirliftConstructor().n_actions(game_board);
                        break;
                    default:
                        game_board.broken()=true;
                        game_board.broken_reasons().push_back("[ForcedDiscardConstructor::n_actions()] Card index ("+std::to_string(e)+") isn't an event card");
                        break;
                }
            }
            // Total actions = # of city cards to discard + each achievable action
            return total_actions;
        }
    }
    game_board.broken()=true;
    game_board.broken_reasons().push_back("[ForcedDiscardConstructor::n_actions()] FORCEDDISCARD n_actions() called but didn't return anything");
    return -1;
}

Actions::Action* Actions::ForcedDiscardConstructor::random_action(Board::Board& game_board){
    // Distributes probability equally over cards, and THEN uniformly over those card actions if >1
    
    for(Players::Player p: game_board.get_players()){
        if(p.hand_full()){
            // pick a card at random
            int card_to_discard_or_use = rand() % p.handsize(); 

            // If the random card is one of the city cards...
            if(card_to_discard_or_use<p.hand.size()){
                // Then just return a forced discard action
                return new Actions::ForcedDiscardAction(p,p.hand[card_to_discard_or_use]);
            } else {
                // Otherwise the random card chosen is one of the event cards.
                // In this case use existing constructors to choose a random action to return.
                if(p.event_cards[card_to_discard_or_use - p.hand.size()]==48 && Decks::CARD_NAME(p.event_cards[card_to_discard_or_use - p.hand.size()])=="Quiet Night"){
                    return new Actions::QuietNight(p); // Return an action to use quiet night
                } else if(p.event_cards[card_to_discard_or_use - p.hand.size()]==49 && Decks::CARD_NAME(p.event_cards[card_to_discard_or_use - p.hand.size()])=="Government Grant"){
                    return GovernmentGrantConstructor().random_action(game_board);
                } else if(p.event_cards[card_to_discard_or_use - p.hand.size()]==50 && Decks::CARD_NAME(p.event_cards[card_to_discard_or_use - p.hand.size()])=="Airlift"){
                    return AirliftConstructor().random_action(game_board);
                }
            }
        }
    }
    game_board.broken()=true; // should never get here
    game_board.broken_reasons().push_back("[ForcedDiscardConstructor::random_action()] Forced Discard Constructor has failed to return any action");
    return nullptr;
}

std::vector<Actions::Action*> Actions::ForcedDiscardConstructor::all_actions(Board::Board& game_board){
    std::vector<Actions::Action*> full_list;
    // First fill the list with the discardable city cards
    for(Players::Player p: game_board.get_players()){
        if(p.hand_full()){
            for(int c: p.hand){
                full_list.push_back(new Actions::ForcedDiscardAction(p,c));
            }
            for(int e: p.event_cards){
                if(e==48 && Decks::CARD_NAME(e)=="Quiet Night"){
                    std::vector<Actions::Action*> qn_actions = Actions::QuietNightConstructor().all_actions(game_board);
                    full_list.insert(full_list.end(),qn_actions.begin(),qn_actions.end());
                } else if(e==49 && Decks::CARD_NAME(e)=="Government Grant"){
                    // this is potentially hella wasteful - copying the ~100 ish actions onto a very small vector rather than the other way around
                    // but should only happen once per game so maybe w/e
                    std::vector<Actions::Action*> gg_actions = Actions::GovernmentGrantConstructor().all_actions(game_board);
                    full_list.insert(full_list.end(),gg_actions.begin(),gg_actions.end());
                } else if(e==50 && Decks::CARD_NAME(e)=="Airlift"){
                    std::vector<Actions::Action*> al_actions = Actions::AirliftConstructor().all_actions(game_board);
                    full_list.insert(full_list.end(),al_actions.begin(),al_actions.end());
                }   
            }
            return full_list; 
        }
    }
    game_board.broken()=true;
    game_board.broken_reasons().push_back("[ForcedDiscardConstructor::all_actions()] FORCEDDISCARD all_actions() called but didn't return anything");
    return {nullptr};
}

bool Actions::ForcedDiscardConstructor::legal(Board::Board& game_board){
    // Should ONLY return legality when there's at least one player with>7 cards in their hand
    // It DOESN't care what phase of the game it is or whose turn it is. This forces an immediate decision node
    for(Players::Player& p: game_board.get_players()){
        if(p.hand_full()){
            return true;
        }
    }
    return false;
}