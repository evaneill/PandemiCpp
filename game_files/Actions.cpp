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
    // If either is being fed a last_position=-1
    if(city_idx1<0 || city_idx2<0){
        // Then there is no neighbor
        return false;
    }
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
// ========================

// ===== ACTION PARENT CLASS =====
Actions::Action::Action(Board::Board& _active_board){
    active_board = &_active_board;
}

// void Actions::Action::execute(){
//     active_board -> broken() = true;
//     active_board -> broken_reasons().push_back("Action parent class execute() called rather than child");  
// }

// ========================

// ===== ACTION CONSTRUCTOR PARENT CLASS =====
Actions::ActionConstructor::ActionConstructor(Board::Board& _active_board){
    active_board = &_active_board;
}
// ========================

// ===== MOVE =====
Actions::Move::Move(Board::Board& _active_board, Map::City _to): Action(_active_board), to(_to){
    movetype = "MOVE";
}

void Actions::Move::execute(){
    Players::Player& active_player = active_board ->active_player();
    // Set last_position tracker to current position index
    active_player.reset_last_position(active_player.get_position().index);

    active_player.set_position(to);

    active_board ->get_turn_action()++;
}

std::string Actions::Move::repr(){
    return movetype+" to "+to.name;
}

Actions::MoveConstructor::MoveConstructor(Board::Board& _active_board): ActionConstructor(_active_board){}

std::string Actions::MoveConstructor::get_movetype(){
    return movetype;
}

int Actions::MoveConstructor::n_actions(){
    if(legal()){
        std::vector<int> legal_moves = {};
        for(int& neighbor: active_board -> active_player().get_position().neighbors){
            if(neighbor!=active_board->active_player().get_last_position() || active_board->active_player().get_position().index==17){
                legal_moves.push_back(neighbor);
            }
        }
        return legal_moves.size();
    } else{
        return 0;
    }
}

Actions::Action* Actions::MoveConstructor::random_action(){
    Players::Player& active_player = active_board ->active_player();
    std::vector<int> legal_moves={};
    for(int neighbor: active_player.get_position().neighbors){
        // make an exception for being in santiago to prevent being trapped for the rest of the turn
        // (Santiago has ONE neighbor, and preventing a move back to it is slightly suboptimal)
        if(neighbor!=active_board->active_player().get_last_position() || active_player.get_position().index==17){
            legal_moves.push_back(neighbor);
        }
    }
    int chosen_neighbor = rand() % legal_moves.size();
    return new Actions::Move(*active_board,Map::CITIES[legal_moves[chosen_neighbor]]);
}

std::vector<Actions::Action*> Actions::MoveConstructor::all_actions(){
    std::vector<int> neighbors = active_board ->active_player().get_position().neighbors;
    std::vector<Actions::Action*> full_list;
    for(int neighbor: active_board->active_player().get_position().neighbors){
        if(neighbor!=active_board->active_player().get_last_position() || active_board -> active_player().get_position().index==17){
            full_list.push_back(new Actions::Move(*active_board,Map::CITIES[neighbor]));
        }
    }
    return full_list;
}

bool Actions::MoveConstructor::legal(){
    // If player has expended 4 moves they can't move, otherwise they can.
    if((active_board ->get_turn_action())<4){
        return true;
    } else {
        return false;
    }
}

// ========================

// ===== DIRECTFLIGHT =====
Actions::DirectFlight::DirectFlight(Board::Board& _active_board,int city_idx): 
    Action(_active_board), 
    citycard(Decks::CityCard(city_idx)){
    movetype = "DIRECTFLIGHT";
}

void Actions::DirectFlight::execute(){
    Players::Player& active_player = active_board ->active_player();

    active_player.reset_last_position(active_player.get_position().index);

    // Set position to city represented by card
    active_player.set_position(Map::CITIES[citycard.index]);

    // Remove from player hand
    active_player.removeCard(citycard);

    active_board ->get_turn_action()++;
}

std::string Actions::DirectFlight::repr(){
    return movetype+" to "+citycard.name;
}

Actions::DirectFlightConstructor::DirectFlightConstructor(Board::Board& _active_board): ActionConstructor(_active_board){}

std::string Actions::DirectFlightConstructor::get_movetype(){
    return movetype;
}

int Actions::DirectFlightConstructor::n_actions(){
    if(legal()){
        Players::Player& active_player = active_board -> active_player();

        int n_actions=0;
        for(Decks::PlayerCard& card: active_player.hand){
            if(!isneighbor(card.index,active_player.get_position().index) && card.index!=active_player.get_position().index && card.index!=active_player.get_last_position()){
                n_actions++;
            }
        }
        return n_actions;
    } else { 
        return 0;
    }

}
Actions::Action* Actions::DirectFlightConstructor::random_action(){
    Players::Player& active_player = active_board ->active_player();
    std::vector<int> neighbors = active_player.get_position().neighbors;
    int chosen_card = rand() % active_player.hand.size();

    while(isneighbor(active_player.hand[chosen_card].index,active_player.get_position().index) || active_player.hand[chosen_card].index==active_player.get_position().index || active_player.hand[chosen_card].index==active_player.get_last_position()){
        // make sure it's not a neighbor, current position, or last position
        chosen_card = rand() % active_player.hand.size();
    }
    return new Actions::DirectFlight(*active_board,active_player.hand[chosen_card].index);
}

std::vector<Actions::Action*> Actions::DirectFlightConstructor::all_actions(){
    std::vector<Actions::Action*> full_list = {};

    Players::Player& active_player = active_board -> active_player();

    for(Decks::PlayerCard& card: active_player.hand){
        if(!isneighbor(card.index,active_player.get_position().index) && card.index!=active_player.get_position().index && card.index!=active_player.get_last_position()){
            full_list.push_back(new Actions::DirectFlight(*active_board,card.index));
        }
    }
    return full_list;
}

bool Actions::DirectFlightConstructor::legal(){
    std::vector<Decks::PlayerCard>& player_hand = active_board ->active_player().hand;
    // If it's during the player turn phase and they have city cards to discard, then it's legal
    if(active_board ->get_turn_action()<4){
        Players::Player& active_player = active_board -> active_player();

        for(Decks::PlayerCard& card: active_player.hand){
            if(!isneighbor(card.index,active_player.get_position().index) 
                && card.index!=active_player.get_position().index 
                && card.index!=active_player.get_last_position()){
                return true;
            }
        }
    } 
    return false;
}
// ========================

// ===== CHARTERFLIGHT =====
Actions::CharterFlight::CharterFlight(Board::Board& _active_board,int _target_city): 
    Action(_active_board), 
    target_city(Map::CITIES[_target_city]){
    movetype = "CHARTERFLIGHT";
}

Actions::CharterFlight::CharterFlight(Board::Board& _active_board,Map::City _target_city): Action(_active_board), target_city(_target_city){
    movetype = "CHARTERFLIGHT";
}

void Actions::CharterFlight::execute(){
    Players::Player& active_player = active_board ->active_player();

    // Set last_position to previous position
    active_player.reset_last_position(active_player.get_position().index);

    // Remove from player hand
    active_player.removeCard(Decks::CityCard(active_player.get_position()));

    // Set position to city represented by card
    active_player.set_position(target_city);

    active_board ->get_turn_action()++;
}

std::string Actions::CharterFlight::repr(){
    return movetype+" to "+target_city.name;
}

Actions::CharterFlightConstructor::CharterFlightConstructor(Board::Board& _active_board): ActionConstructor(_active_board){}

std::string Actions::CharterFlightConstructor::get_movetype(){
    return movetype;
}

int Actions::CharterFlightConstructor::n_actions(){
    if(legal()){
        Players::Player& active_player = active_board -> active_player();
        if(active_player.get_last_position()>=0 && !isneighbor(active_player.get_last_position(),active_player.get_last_position())){
            // If the last position is set and it isn't a neighbor, then we can't go to it (-1), our current position (-1), or any neighbor (-neighbors.size())
            return Map::CITIES.size()-2-active_player.get_position().neighbors.size();
        } else {
            // Otherwise there's either no last_position or it's a neighbor. In either case remove all neighbors and current position
            return Map::CITIES.size() - 1 - active_player.get_position().neighbors.size();
        }
    } else{
        return 0;
    }
    
}

Actions::Action* Actions::CharterFlightConstructor::random_action(){
    Players::Player& active_player = active_board ->active_player();
    std::vector<int> neighbors = active_player.get_position().neighbors;

    int random_position = rand() % Map::CITIES.size(); // choose a random city to go to
    while(isneighbor(random_position,active_player.get_position().index) || random_position==active_player.get_position().index || random_position==active_player.get_last_position()){
        random_position = rand() % Map::CITIES.size(); 
    }
    return new Actions::CharterFlight(*active_board,random_position);
}

std::vector<Actions::Action*> Actions::CharterFlightConstructor::all_actions(){
    std::vector<Actions::Action*> full_list;
    Players::Player& active_player = active_board -> active_player();
    for(int n;n<Map::CITIES.size();n++){
        if(!isneighbor(n,active_player.get_position().index) && n!=active_player.get_position().index && n!=active_player.get_last_position()){
            full_list.push_back(new Actions::CharterFlight(*active_board,Map::CITIES[n]));
        }
    }
    // Get rid of the position of the player. (Left out for now)
    // full_list.erase(full_list.begin()+active_board ->active_player().get_position().index);
    return full_list;
}

bool Actions::CharterFlightConstructor::legal(){
    Players::Player& active_player = active_board ->active_player();
    // If it's the player turn phase and they have the card that matches the city they're in, return true
    for(Decks::PlayerCard c: active_player.hand){
        if(c.index==active_player.get_position().index){
            return true;
        }
    }
    // But if no card matches their position return false
    return false;
}
// ========================

// ===== SHUTTLEFLIGHT =====
Actions::ShuttleFlight::ShuttleFlight(Board::Board& _active_board,int _target_station): 
    Action(_active_board), 
    target_station(Map::CITIES[_target_station]){
    movetype = "SHUTTLEFLIGHT";
}

Actions::ShuttleFlight::ShuttleFlight(Board::Board& _active_board,Map::City _target_station): Action(_active_board), target_station(_target_station){
    movetype = "SHUTTLEFLIGHT";
}

void Actions::ShuttleFlight::execute(){
    Players::Player& active_player = active_board ->active_player();

    active_player.reset_last_position(active_player.get_position().index);

    // Set position to city represented by target_station
    active_player.set_position(target_station);

    active_board ->get_turn_action()++;
}

std::string Actions::ShuttleFlight::repr(){
    return movetype+" to "+target_station.name;
}

Actions::ShuttleFlightConstructor::ShuttleFlightConstructor(Board::Board& _active_board): ActionConstructor(_active_board){}

std::string Actions::ShuttleFlightConstructor::get_movetype(){
    return movetype;
}

int Actions::ShuttleFlightConstructor::n_actions(){
    if(legal()){
        int n_actions=0;
        for(Map::City st: active_board -> get_stations()){
            if(st.index!=active_board->active_player().get_position().index && st.index!=active_board->active_player().get_last_position()){
                n_actions++;
            }
        }
        return n_actions;
    } else {
        return 0;
    }
}

Actions::Action* Actions::ShuttleFlightConstructor::random_action(){
    int position  = active_board ->active_player().get_position().index;
    int random_position = rand() % active_board ->get_stations().size();
    while((*active_board).get_stations()[random_position].index==position || (*active_board).get_stations()[random_position].index==active_board->active_player().get_last_position()){
        // The shuttle flight action says you can go to "any _other_ city" with a station. I interpret this to exclude the current city, unlike other actions.
        random_position = rand() % active_board ->get_stations().size(); 
    }
    return new Actions::ShuttleFlight(*active_board,active_board -> get_stations()[random_position]);
}

std::vector<Actions::Action*> Actions::ShuttleFlightConstructor::all_actions(){
    std::vector<Actions::Action*> full_list;
    std::vector<Map::City> stations = active_board ->get_stations();

    for(Map::City& st: stations){
        if(st.index!=active_board ->active_player().get_position().index && st.index!=active_board -> active_player().get_last_position()){
            full_list.push_back(new Actions::ShuttleFlight(*active_board,st));
        } 
    }
    return full_list;
}

bool Actions::ShuttleFlightConstructor::legal(){
    Players::Player& active_player = active_board ->active_player();
    // if it's the player turn
    if(active_board ->get_turn_action()<4){
        // and there's at least 2 stations
        if(active_board ->get_stations().size()>1){
            for(Map::City st: active_board ->get_stations()){
                // and the player is at a station
                if(st.index==active_player.get_position().index){
                    for(Map::City other_st : active_board-> get_stations()){
                        // and at least one of the other stations isn't their previous position...
                        if(other_st.index!=active_player.get_last_position() && other_st.index!=st.index){
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
Actions::OperationsExpertFlight::OperationsExpertFlight(Board::Board& _active_board,int _target_city,Decks::CityCard _discard): 
    Action(_active_board), 
    target_city(Map::CITIES[_target_city]),
    discard_card(_discard)
    {
    movetype = "OPERATIONSEXPERTFLIGHT";
}

Actions::OperationsExpertFlight::OperationsExpertFlight(Board::Board& _active_board,int _target_city,int _discard_city_idx): 
    Action(_active_board), 
    target_city(Map::CITIES[_target_city]),
    discard_card(Decks::CityCard(_discard_city_idx))
    {
    movetype = "OPERATIONSEXPERTFLIGHT";
}

Actions::OperationsExpertFlight::OperationsExpertFlight(Board::Board& _active_board,Map::City _target_city,Decks::CityCard _discard): 
    Action(_active_board), 
    target_city(_target_city),
    discard_card(_discard) 
    {
    movetype = "OPERATIONSEXPERTFLIGHT";
}

void Actions::OperationsExpertFlight::execute(){
    Players::Player& active_player = active_board ->active_player();

    active_player.reset_last_position(active_player.get_position().index);

    active_player.used_OperationsExpertFlight=true;

    // Remove discard_card from hand
    active_player.removeCard(discard_card);

    // Set position to city represented by target_station
    active_player.set_position(target_city);

    active_board ->get_turn_action()++;
}

std::string Actions::OperationsExpertFlight::repr(){
    return movetype+" to "+target_city.name+" (discard "+discard_card.name+")";
}

Actions::OperationsExpertFlightConstructor::OperationsExpertFlightConstructor(Board::Board& _active_board): ActionConstructor(_active_board){}

std::string Actions::OperationsExpertFlightConstructor::get_movetype(){
    return movetype;
}

int Actions::OperationsExpertFlightConstructor::n_actions(){
    if(legal()){
        Players::Player& active_player = active_board -> active_player();
        if(active_player.get_last_position()>=0 && !isneighbor(active_player.get_last_position(),active_player.get_last_position())){
            // If the last position is set and it isn't a neighbor, then we can't go to it (-1), our current position (-1), or any neighbor (-neighbors.size()). multiply by hand size (# of discards)
            return (Map::CITIES.size()-2-active_player.get_position().neighbors.size()) * active_player.hand.size();
        } else {
            // Otherwise there's either no last_position or it's a neighbor. In either case remove all neighbors and current position. multiply by hand size (# of discards)
            return (Map::CITIES.size() - 1 - active_player.get_position().neighbors.size()) * active_player.hand.size();
        }
    } else {
        return 0;
    }
    
}

Actions::Action* Actions::OperationsExpertFlightConstructor::random_action(){
    Players::Player& active_player = active_board -> active_player();
    
    int position  = active_player.get_position().index;
    
    // Random destination
    int random_position = rand() % Map::CITIES.size();
    while(random_position==position || random_position==active_player.get_last_position() || isneighbor(random_position,position)){
        random_position = rand() % Map::CITIES.size();
    }

    // Randomly discarded city card
    int random_discard = rand() % active_board ->active_player().hand.size();

    return new Actions::OperationsExpertFlight(*active_board,random_position,Decks::CityCard(active_board -> active_player().hand[random_discard].index));
}

std::vector<Actions::Action*> Actions::OperationsExpertFlightConstructor::all_actions(){
    std::vector<Actions::Action*> full_list;
    std::vector<Decks::PlayerCard>& hand = active_board ->active_player().hand;
    Players::Player& active_player = active_board -> active_player();

    for(int n;n<Map::CITIES.size();n++){
        if(!isneighbor(n,active_player.get_position().index) && n!=active_player.get_position().index && n!=active_player.get_last_position()){
            for(int c=0; c<hand.size();c++){
                full_list.push_back(new Actions::OperationsExpertFlight(*active_board,n,hand[c].index));
            } 
        }
    }
    return full_list;
}

bool Actions::OperationsExpertFlightConstructor::legal(){
    Players::Player& active_player = active_board ->active_player();
    // If active player is the operations expert
    if(active_player.role.name=="Operations Expert"){
        // and it's still players turn to do stuff
        if(active_board ->get_turn_action()<4){
            for(Map::City st: active_board ->get_stations()){
                // And they're at the position of a station
                if(st.index==active_player.get_position().index){
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
Actions::Build::Build(Board::Board& _active_board,int _remove_station): 
    Action(_active_board),
    remove_station(_remove_station)
    {
    movetype = "BUILD";

}

void Actions::Build::execute(){
    Players::Player& active_player = active_board ->active_player();
    
    // Player has "done something" -> now it doesn't matter what the last position was
    active_player.reset_last_position();

    // If the remove_station argument>=0 and so representing a station to remove
    if(remove_station>=0){
        // Should break catastrophically when remove_station isn't defined properly
        // (Should be index of station to remove in active_board.stations)
        active_board -> RemoveStation(remove_station);
    }

    // Add it to the vector of research station Map::City elements
    active_board -> AddStation(Map::CITIES[active_player.get_position().index]);

    //Check whether active player is Operations Expert. If not, discard necessary card.
    if(active_player.role.name!="Operations Expert"){
        active_player.removeCard(Decks::CityCard(active_player.get_position()));
    }

    active_board ->get_turn_action()++;
}

std::string Actions::Build::repr(){
    std::string str_out = movetype+" at "+active_board ->active_player().get_position().name;
    if(remove_station>=0){
        return str_out + " (remove the one at " + Map::CITIES[active_board -> get_stations()[remove_station].index].name + ")";
    } else {
        return str_out;
    }
}

Actions::BuildConstructor::BuildConstructor(Board::Board& _active_board): ActionConstructor(_active_board){}

std::string Actions::BuildConstructor::get_movetype(){
    return movetype;
}

int Actions::BuildConstructor::n_actions(){
    if(legal()){
        if(active_board ->get_stations().size()>=6){
            return active_board ->get_stations().size(); // Can remove any existing stations. (In a vanilla game this can't be more than 6)
        } else {
            return 1; // otherwise there's only one thing to do
        }
    } else {
        return 0;
    }
    
}

Actions::Action* Actions::BuildConstructor::random_action(){
    if(active_board ->get_stations().size()>=6){
        return new Actions::Build(*active_board,rand() % active_board ->get_stations().size());
    } else {
        return new Actions::Build(*active_board,-1);
    }
}

std::vector<Actions::Action*> Actions::BuildConstructor::all_actions(){
    std::vector<Actions::Action*> full_list;
    std::vector<Map::City> stations = active_board ->get_stations();

    if(stations.size()>=6){
        // Should never be >6 but just in case...
        for(int st=0;st<stations.size();st++){
            // If this station isn't at your current location (shouldn't be possible when guarded by legal())
            if(stations[st].index!=active_board ->active_player().get_position().index){
                full_list.push_back(new Actions::Build(*active_board,st));
            } 
        }
    } else {
        full_list.push_back(new Actions::Build(*active_board));
    }
    return full_list;
}

bool Actions::BuildConstructor::legal(){
    Players::Player& active_player = active_board ->active_player();
    if(active_board ->get_turn_action()<4){
        if(active_player.role.name=="Operations Expert"){
            bool already_station = false;
            for(Map::City st: active_board ->get_stations()){
                if(st.index==active_player.get_position().index){
                    already_station=true;
                    break;
                }
            }
            if(!already_station){
                return true;
            }
        } else{
            for(Decks::PlayerCard c : active_player.hand){
                // Then if there's a card representing the city they're in...
                if(c.index==active_player.get_position().index){
                    bool already_station = false;
                    // If no station is already on this city..
                    for(Map::City st: active_board ->get_stations()){
                        if(st.index==active_player.get_position().index){
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
Actions::Treat::Treat(Board::Board& _active_board,int _color): 
    Action(_active_board), 
    color(_color){
    movetype = "TREAT";
}

void Actions::Treat::execute(){
    Players::Player& active_player = active_board ->active_player();
    
    // Player has "done something" -> now it doesn't matter what the last position was
    active_player.reset_last_position();
    
    if(active_player.role.name=="Medic" || active_board -> is_cured(color)){
        active_board ->get_disease_count()[color][active_player.get_position().index]=0;
    } else {
        active_board ->get_disease_count()[color][active_player.get_position().index]--;
    }
    // Check for eradication
    // If(this color is cured && there are now 0 cubes of this disease)
    std::array<int,48>& disease_count = active_board ->get_disease_count()[color];
    if(active_board -> is_cured(color) && std::accumulate(disease_count.begin(),disease_count.end(),0)==0){
        active_board ->Eradicate(color);
    }

    active_board ->get_turn_action()++;
}

std::string Actions::Treat::repr(){
    Players::Player& active_player = active_board ->active_player();
    int n_treated;
    if(active_player.role.name=="Medic" || active_board ->is_cured(color)){
        n_treated = active_board ->get_disease_count()[color][active_player.get_position().index];
    } else {
        n_treated = 1;
    }
    return movetype+" " + std::to_string(n_treated) +" "+ Map::COLORS[color] +" on "+active_player.get_position().name;
}

Actions::TreatConstructor::TreatConstructor(Board::Board& _active_board): ActionConstructor(_active_board){}

std::string Actions::TreatConstructor::get_movetype(){
    return movetype;
}

int Actions::TreatConstructor::n_actions(){
    if(legal()){
        int n_actions = 0;
        Players::Player& active_player = active_board ->active_player();
        if(active_board ->get_disease_count()[Map::BLUE][active_player.get_position().index]>0){
            n_actions++;
        }
        if(active_board ->get_disease_count()[Map::YELLOW][active_player.get_position().index]>0){
            n_actions++;
        }
        if(active_board ->get_disease_count()[Map::BLACK][active_player.get_position().index]>0){
            n_actions++;
        }
        if(active_board ->get_disease_count()[Map::RED][active_player.get_position().index]>0){
            n_actions++;
        }
        return n_actions; // 1 for each color of disease on the position of the current city
    } else {
        return 0;
    }
     
}

Actions::Action* Actions::TreatConstructor::random_action(){
    std::vector<int> nonzero_colors = {};
    Players::Player& active_player = active_board ->active_player();
    if(active_board ->get_disease_count()[Map::BLUE][active_player.get_position().index]>0){
        nonzero_colors.push_back(Map::BLUE);
    }
    if(active_board ->get_disease_count()[Map::YELLOW][active_player.get_position().index]>0){
        nonzero_colors.push_back(Map::YELLOW);
    }
    if(active_board ->get_disease_count()[Map::BLACK][active_player.get_position().index]>0){
        nonzero_colors.push_back(Map::BLACK);
    }
    if(active_board ->get_disease_count()[Map::RED][active_player.get_position().index]>0){
        nonzero_colors.push_back(Map::RED);
    }
    return new Actions::Treat(*active_board,nonzero_colors[rand() % nonzero_colors.size()]);
}

std::vector<Actions::Action*> Actions::TreatConstructor::all_actions(){
    std::vector<Actions::Action*> full_list;
    Players::Player& active_player = active_board ->active_player();
    if((*active_board).get_disease_count()[Map::BLUE][active_player.get_position().index]>0){
        full_list.push_back(new Actions::Treat(*active_board,Map::BLUE));
    }
    // If I do it this way, then the fast-compiled version won't decide to skip a loop?
    if((*active_board).get_disease_count()[Map::YELLOW][active_player.get_position().index]>0){
        full_list.push_back(new Actions::Treat(*active_board,Map::YELLOW));
    }
    // If I do it this way, then the fast-compiled version won't decide to skip a loop?
    if((*active_board).get_disease_count()[Map::BLACK][active_player.get_position().index]>0){
        full_list.push_back(new Actions::Treat(*active_board,Map::BLACK));
    }
    // If I do it this way, then the fast-compiled version won't decide to skip a loop?
    if((*active_board).get_disease_count()[Map::RED][active_player.get_position().index]>0){
        full_list.push_back(new Actions::Treat(*active_board,Map::RED));
    }
    return full_list;
}

bool Actions::TreatConstructor::legal(){
    // If it's a players turn...
    if(active_board -> get_turn_action()<4){
        Players::Player& active_player = active_board ->active_player();
        // And there's a color that has >0 cubes at the present city, it's legal
        
        // If I do it this way, then the fast-compiled version won't decide to skip a loop?
        if((*active_board).get_disease_count()[Map::BLUE][active_player.get_position().index]>0){
            return true;
        }
        // If I do it this way, then the fast-compiled version won't decide to skip a loop?
        if((*active_board).get_disease_count()[Map::YELLOW][active_player.get_position().index]>0){
            return true;
        }
        // If I do it this way, then the fast-compiled version won't decide to skip a loop?
        if((*active_board).get_disease_count()[Map::BLACK][active_player.get_position().index]>0){
            return true;
        }
        // If I do it this way, then the fast-compiled version won't decide to skip a loop?
        if((*active_board).get_disease_count()[Map::RED][active_player.get_position().index]>0){
            return true;
        }
    }
    return false;
}
// ========================

// ===== CURE =====
Actions::Cure::Cure(Board::Board& _active_board): 
    Action(_active_board){
    movetype = "CURE";
}

void Actions::Cure::execute(){
    Players::Player& active_player = active_board ->active_player();

    // Player has "done something" -> now it doesn't matter what the last position was
    active_player.reset_last_position();

    for(Map::City st: active_board -> get_stations()){
        if(st.index==active_player.get_position().index){
            std::array<int,4> color_count = active_player.get_color_count();

            if(color_count[Map::BLUE]>=active_player.role.required_cure_cards && !active_board -> is_cured(Map::BLUE)){
                active_player.removeCureCardColor(Map::BLUE);
                active_board -> Cure(Map::BLUE);
                active_board -> get_turn_action()++;
                // Update eradicated status if there is no disease of this color on the board
                if(std::accumulate(active_board -> get_disease_count()[Map::BLUE].begin(),active_board -> get_disease_count()[Map::BLUE].begin(),0)==0){
                    active_board ->Eradicate(Map::BLUE);
                }
                return;
            }
            if(color_count[Map::YELLOW]>=active_player.role.required_cure_cards && !active_board -> is_cured(Map::YELLOW)){
                active_player.removeCureCardColor(Map::YELLOW);
                active_board -> Cure(Map::YELLOW);
                active_board -> get_turn_action()++;
                // Update eradicated status if there is no disease of this color on the board
                if(std::accumulate(active_board -> get_disease_count()[Map::YELLOW].begin(),active_board -> get_disease_count()[Map::YELLOW].begin(),0)==0){
                    active_board ->Eradicate(Map::YELLOW);
                }
                return;
            }
            if(color_count[Map::BLACK]>=active_player.role.required_cure_cards && !active_board -> is_cured(Map::BLACK)){
                active_player.removeCureCardColor(Map::BLACK);
                active_board -> Cure(Map::BLACK);
                active_board -> get_turn_action()++;
                // Update eradicated status if there is no disease of this color on the board
                if(std::accumulate(active_board -> get_disease_count()[Map::BLACK].begin(),active_board -> get_disease_count()[Map::BLACK].begin(),0)==0){
                    active_board ->Eradicate(Map::BLACK);
                }
                return;
            }
            if(color_count[Map::RED]>=active_player.role.required_cure_cards && !active_board -> is_cured(Map::RED)){
                active_player.removeCureCardColor(Map::RED);
                active_board -> Cure(Map::RED);
                active_board -> get_turn_action()++;
                // Update eradicated status if there is no disease of this color on the board
                if(std::accumulate(active_board -> get_disease_count()[Map::RED].begin(),active_board -> get_disease_count()[Map::RED].begin(),0)==0){
                    active_board ->Eradicate(Map::RED);
                }
                return;
            }
        }
    }
    active_board ->broken()=true;
    active_board ->broken_reasons().push_back("[Cure::execute()] CURE execute() was called but there don't seem to be enough of any card to cure, or they weren't at a research station...");
}

std::string Actions::Cure::repr(){
    Players::Player& active_player = active_board ->active_player();

    std::array<int,4> color_count = active_player.get_color_count();

    if(color_count[Map::BLUE]>=active_player.role.required_cure_cards && !active_board -> is_cured(Map::BLUE)){
        return movetype+" "+Map::COLORS[Map::BLUE];
    }
    if(color_count[Map::YELLOW]>=active_player.role.required_cure_cards && !active_board -> is_cured(Map::YELLOW)){
        return movetype+" "+Map::COLORS[Map::YELLOW];
    }
    if(color_count[Map::BLACK]>=active_player.role.required_cure_cards && !active_board -> is_cured(Map::BLACK)){
        return movetype+" "+Map::COLORS[Map::BLACK];
    }
    if(color_count[Map::RED]>=active_player.role.required_cure_cards && !active_board -> is_cured(Map::RED)){
        return movetype+" "+Map::COLORS[Map::RED];
    }
    active_board ->broken()=true;
    active_board ->broken_reasons().push_back("[Cure::repr()] CURE repr() was called but there either weren't enough cards or the disease was already cured...");
}

Actions::CureConstructor::CureConstructor(Board::Board& _active_board): ActionConstructor(_active_board){}

std::string Actions::CureConstructor::get_movetype(){
    return movetype;
}

int Actions::CureConstructor::n_actions(){
    if(legal()){
        return 1; //  If this is available, there's only 1 curable disease by the active player.
    } else {
        return 0;
    }
}

Actions::Action* Actions::CureConstructor::random_action(){
    return new Actions::Cure(*active_board);// if it's legal, then there's only one choice.
}

std::vector<Actions::Action*> Actions::CureConstructor::all_actions(){
    std::vector<Actions::Action*> full_list;
    full_list.push_back(new Actions::Cure(*active_board));
    return full_list; // Like above; if it's legal then there's only one action: list of size 1
}

bool Actions::CureConstructor::legal(){
    // If it's a players turn...
    if(active_board ->get_turn_action()<4){
        Players::Player& active_player = active_board ->active_player();

        std::array<int,4> color_count = active_player.get_color_count();

        // and they have the required cards for an uncured color        
        if((color_count[Map::BLUE]>=active_player.role.required_cure_cards && !active_board -> is_cured(Map::BLUE)) ||
            (color_count[Map::YELLOW]>=active_player.role.required_cure_cards && !active_board -> is_cured(Map::YELLOW)) ||
            (color_count[Map::BLACK]>=active_player.role.required_cure_cards && !active_board -> is_cured(Map::BLACK)) ||
            (color_count[Map::RED]>=active_player.role.required_cure_cards && !active_board -> is_cured(Map::RED))){
            for(Map::City st: active_board -> get_stations()){
                // And they're at a research station...
                if(st.index==active_player.get_position().index){
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
Actions::Give::Give(Board::Board& _active_board, Players::Player& _other_player, Decks::CityCard _card_to_give):
    Action(_active_board),
    other_player(_other_player),
    card_to_give(_card_to_give)
    {
    movetype = "GIVE";
}

Actions::Give::Give(Board::Board& _active_board, Players::Player& _other_player, int _card_to_give_cityidx):
    Action(_active_board),
    other_player(_other_player),
    card_to_give(Decks::CityCard(_card_to_give_cityidx))
    {
    movetype = "GIVE";
}

void Actions::Give::execute(){
    Players::Player& active_player = active_board ->active_player();

    // Player has "done something" -> now it doesn't matter what the last position was
    active_player.reset_last_position();

    active_player.removeCard(card_to_give);

    other_player.UpdateHand(card_to_give);
    active_board ->get_turn_action()++;
}

std::string Actions::Give::repr(){
    Players::Player& active_player = active_board ->active_player();
    return movetype + " " + card_to_give.name + " to " + other_player.role.name;
}

Actions::GiveConstructor::GiveConstructor(Board::Board& _active_board): ActionConstructor(_active_board){}

std::string Actions::GiveConstructor::get_movetype(){
    return movetype;
}

int Actions::GiveConstructor::n_actions(){
    if(legal()){
        Players::Player& active_player = active_board ->active_player();
        // Can only ever be # of other players on this city, otherwise whole hand * # of other players on this city
        int n_other_players_here = 0; 
        // For every player, check whether they're in the same position as active_player but aren't active_player
        for(Players::Player& _other_player: active_board -> get_players()){
            // If they're in the same city...
            if(_other_player.get_position().index==active_player.get_position().index && active_player.role.name!=_other_player.role.name){
                n_other_players_here++;
            }
        }
        if(active_player.role.name=="Researcher"){
            return active_player.hand.size()*n_other_players_here;
        } else {
            return n_other_players_here;
        }
    } else {
        return 0;
    }  
}

Actions::Action* Actions::GiveConstructor::random_action(){
    Players::Player& active_player = active_board ->active_player();
    Decks::CityCard card_to_give = NULL;

    // First randomize the card that active_player will give based on their role
    if(active_player.role.name=="Researcher"){
        card_to_give = Decks::CityCard(active_player.hand[rand() % active_player.hand.size()].index);
    } else {
        card_to_give = Decks::CityCard(active_player.get_position());
    }

    // Check how many other players are here.
    int n_other_players_here = 0; // initialize at -1 since we'll iterate through all players and see active_player at current city
    for(Players::Player& _other_player: active_board -> get_players()){
        // If they're in the same city...
        if(_other_player.get_position().index==active_player.get_position().index && active_player.role.name!=_other_player.role.name){
            n_other_players_here++;
        }
    }

    // Randomized value to choose which other player at this city we'll give to
    int which_player = rand() % n_other_players_here; // 0 ... (# of other players in city - 1)
    // Incremented value to track which of the other players on this city we're considering
    int track=0;
    for(Players::Player& _other_player: active_board -> get_players()){
        // If the other player is here and isn't the active_player...
        if(_other_player.get_position().index==active_player.get_position().index && active_player.role.name!=_other_player.role.name){
            // If track== the random value generated, return an action!
            if(track==which_player){
                return new Actions::Give(*active_board, _other_player,card_to_give.index);
            }
            // Otherwise increment up track; it's a different player we want at this city.
            track++;
        }
    }
    active_board ->broken()=true;
    active_board ->broken_reasons().push_back("[GiveConstructor::random_action()] GIVE failed to return a random action!");
}

std::vector<Actions::Action*> Actions::GiveConstructor::all_actions(){
    std::vector<Actions::Action*> full_list;
    Players::Player& active_player = active_board ->active_player();
    std::vector<Decks::PlayerCard> cards_to_give = {};

    if(active_player.role.name=="Researcher"){
        cards_to_give = active_player.hand;
    } else {
        cards_to_give = {Decks::CityCard(active_player.get_position())};
    }

    for(int p=0;p<active_board ->get_players().size();p++){
        Players::Player& _other_player = active_board ->get_players()[p];
        if(_other_player.get_position().index==active_player.get_position().index && active_player.role.name!=_other_player.role.name){
            for(int c=0;c<cards_to_give.size();c++){
                full_list.push_back(new Actions::Give(*active_board,_other_player,cards_to_give[c].index));
            }
        }
    }

    return full_list;
}

bool Actions::GiveConstructor::legal(){
    if(active_board ->get_turn_action()<4){
        Players::Player& active_player = active_board ->active_player();
        for(Players::Player& p: active_board ->get_players()){
            if(p.get_position().index==active_player.get_position().index && active_player.role.name!=p.role.name){
                if(active_player.hand.size()>0){
                    if(active_player.role.name=="Researcher"){
                        return true;
                    } else {
                        for(Decks::PlayerCard card: active_player.hand){
                            if(card.index==active_player.get_position().index){
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
Actions::Take::Take(Board::Board& _active_board, Players::Player& _other_player, Decks::CityCard _card_to_take):
    Action(_active_board),
    other_player(_other_player),
    card_to_take(_card_to_take)
    {
    movetype = "TAKE";
}

Actions::Take::Take(Board::Board& _active_board, Players::Player& _other_player, int _card_to_take_city_idx):
    Action(_active_board),
    other_player(_other_player),
    card_to_take(Decks::CityCard(_card_to_take_city_idx))
    {
    movetype = "TAKE";
}

void Actions::Take::execute(){
    Players::Player& active_player = active_board ->active_player();

    // Player has "done something" -> now it doesn't matter what the last position was
    active_player.reset_last_position();

    other_player.removeCard(card_to_take);

    active_player.UpdateHand(card_to_take);
    active_board ->get_turn_action()++;
}

std::string Actions::Take::repr(){
    Players::Player& active_player = active_board ->active_player();
    return movetype + " " + card_to_take.name + " from " + other_player.role.name;
}

Actions::TakeConstructor::TakeConstructor(Board::Board& _active_board): ActionConstructor(_active_board){}

std::string Actions::TakeConstructor::get_movetype(){
    return movetype;
}

int Actions::TakeConstructor::n_actions(){
    if(legal()){
        Players::Player& active_player = active_board ->active_player();
    
        // Have to check each same-city player and add based on researcher status and active_player hand fullness
        int n_actions = 0; 
        // For every player, check whether they're in the same position as active_player but aren't active_player
        for(int p=0;p<active_board ->get_players().size();p++){
            Players::Player& _other_player = active_board ->get_players()[p];
            // If they're in the same city...
            if(_other_player.get_position().index==active_player.get_position().index && active_player.role.name!=_other_player.role.name){
                // and it's the researcher...
                if(_other_player.role.name=="Researcher"){
                    // then we could take any of their cards
                    n_actions+=_other_player.hand.size();
                } else {
                    // We just add 1 if they have a card we could take (the city we're in)
                    for(Decks::PlayerCard c: _other_player.hand){
                        if(c.index==active_player.get_position().index){
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

Actions::Action* Actions::TakeConstructor::random_action(){
    Players::Player& active_player = active_board ->active_player();

    // I tried thinking of a way to do this without calling all actions
    // But that seemed at least as hard as just doing this
    // This gives uniformity over all cards to take
    std::vector<Actions::Action*> action_list = all_actions();
    return action_list[rand() % action_list.size()];
}

std::vector<Actions::Action*> Actions::TakeConstructor::all_actions(){
    std::vector<Actions::Action*> full_list;
    Players::Player& active_player = active_board ->active_player();
    std::vector<Decks::PlayerCard> cards_to_take = {};

    for(Players::Player& _other_player : active_board -> get_players()){
        // If the other player is at active_players position
        if(_other_player.get_position().index==active_player.get_position().index && active_player.role.name!=_other_player.role.name){
            // Then check through each card...
            for(Decks::PlayerCard card: _other_player.hand){
                // And if they're a researcher or the card index is the same as active_players position
                if(card.index==active_player.get_position().index || _other_player.role.name=="Researcher"){
                    // Then add it to the list
                    full_list.push_back(new Actions::Take(*active_board,_other_player,card.index));
                }
            }
        }
    }
    return full_list;
}

bool Actions::TakeConstructor::legal(){
    if(active_board ->get_turn_action()<4){
        Players::Player& active_player = active_board ->active_player();
        for(Players::Player& _other_player: active_board -> get_players()){

            if(_other_player.get_position().index==active_player.get_position().index && active_player.role.name!=_other_player.role.name){
                if(_other_player.role.name=="Researcher"){
                    if(_other_player.hand.size()>0){
                        return true;
                    }
                } else {
                    for(Decks::PlayerCard card: _other_player.hand){
                        if(card.index==active_player.get_position().index){
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
Actions::Airlift::Airlift(Board::Board& _active_board,Players::Player& _using_player, Players::Player& _target_player,int _target_city):
    Action(_active_board),
    using_player(_using_player),
    target_player(_target_player),
    target_city(_target_city) // doesn't have to be referenced
    {
    movetype = "AIRLIFT";
}

void Actions::Airlift::execute(){
    // Target player last_position better be updated
    target_player.reset_last_position(target_player.get_position().index);

    target_player.set_position(Map::CITIES[target_city]);
    using_player.removeCard(Decks::EventCard(50));
}

std::string Actions::Airlift::repr(){
    return movetype +" " +using_player.role.name +" airlifts " + target_player.role.name + " to " + Map::CITIES[target_city].name;
}

Actions::AirliftConstructor::AirliftConstructor(Board::Board& _active_board): ActionConstructor(_active_board){}

std::string Actions::AirliftConstructor::get_movetype(){
    return movetype;
}

int Actions::AirliftConstructor::n_actions(){
    if(legal()){
        // Initialize as "every player can move to every other city but their current position"
        int n_actions= (Map::CITIES.size()-1)*active_board->get_players().size();
        // Go through players and remove either size of neighbors AND one for its last position OR just size of neighbors
        for(Players::Player& p : active_board -> get_players()){
            if(p.get_last_position()>=0 && !isneighbor(p.get_position().index,p.get_last_position())){
                n_actions-=(1+p.get_position().neighbors.size());
            } else {
                n_actions-=(p.get_position().neighbors.size());
            }
        }
        return n_actions;
    } else {
        return 0;
    }
}

Actions::Action* Actions::AirliftConstructor::random_action(){
    Players::Player& _target_player = active_board ->get_players()[rand() % active_board ->get_players().size()];
    // Use dumb loop to find the player that's using this action.
    for(Players::Player& _using_player: active_board -> get_players()){
        // Look for Airlift in the player's event_cards
        for(int e=0;e<_using_player.event_cards.size();e++){
            if(_using_player.event_cards[e].name=="Airlift" && _using_player.event_cards[e].index==50){
                int random_city = rand() % Map::CITIES.size();
                while(random_city==_target_player.get_position().index || isneighbor(random_city,_target_player.get_position().index) || random_city==_target_player.get_last_position()){
                    random_city = rand() % Map::CITIES.size();
                }
                return new Actions::Airlift(*active_board,_using_player,_target_player,random_city);
            }
        }
    }
    active_board ->broken()=true;
    active_board ->broken_reasons().push_back("[AirliftConstructor::random_action()] AIRLIFT random_action() called and didn't return an action");
}

std::vector<Actions::Action*> Actions::AirliftConstructor::all_actions(){
    std::vector<Actions::Action*> full_list;
    for(Players::Player _using_player : active_board -> get_players()){
        // Look for Airlift in the player's event_cards
        for(int e=0;e<_using_player.event_cards.size();e++){
            // If we found the airlift-holder...
            if(_using_player.event_cards[e].name=="Airlift" && _using_player.event_cards[e].index==50){
                // Then for every possible target player...
                for(Players::Player& _target_player : active_board -> get_players()){
                    for(int city=0;city<Map::CITIES.size();city++){
                        // for every city that's not a neighbor of the target player, their current position, or last position...
                        if(!isneighbor(city,_target_player.get_position().index) && city!=_target_player.get_position().index && city!=_target_player.get_last_position()){
                            // add an action.
                            full_list.push_back(new Actions::Airlift(*active_board,_using_player,_target_player,city));
                        }
                    }
                    
                }
            }
        }
    }
    return full_list;
}

bool Actions::AirliftConstructor::legal(){
    // The legality guard is always true when the card is held by a player. 
    for(Players::Player& pl : active_board ->get_players()){
        for(Decks::PlayerCard e : pl.event_cards){
            if(e.name=="Airlift" && e.index==50){
                return true;
            }
        }
    }
    return false;
}
// ========================

// ===== GOVERNMENTGRANT =====
Actions::GovernmentGrant::GovernmentGrant(Board::Board& _active_board,Players::Player& _using_player, int _target_city,int _remove_station):
    Action(_active_board),
    using_player(_using_player),
    target_city(_target_city),
    remove_station(_remove_station)
    {
    movetype = "GOVERNMENTGRANT";
}

void Actions::GovernmentGrant::execute(){
    using_player.removeCard(Decks::EventCard(49));
    
    // Add it to the vector of research station Map::City elements
    active_board -> AddStation(Map::CITIES[target_city]);

    if(remove_station>=0){
        // Should break catastrophically when remove_station isn't defined properly
        // (Should be index of station to remove in active_board ->stations)
        active_board ->RemoveStation(remove_station);
    }

}

std::string Actions::GovernmentGrant::repr(){
    std::string out_str = movetype + " " + using_player.role.name + " put a research station at " + Map::CITIES[target_city].name;
    if(remove_station>=0){
        out_str+=" (removed " + Map::CITIES[remove_station].name + ")";
    }
    return out_str;
}

Actions::GovernmentGrantConstructor::GovernmentGrantConstructor(Board::Board& _active_board): ActionConstructor(_active_board){}

std::string Actions::GovernmentGrantConstructor::get_movetype(){
    return movetype;
}

int Actions::GovernmentGrantConstructor::n_actions(){
    if(legal()){
        if(active_board ->get_stations().size()==6){
            return 6*(Map::CITIES.size()-6); // can place a new station at any of 48-6 locations, and remove any of 6 current stations
        } else
            return Map::CITIES.size() - active_board ->get_stations().size(); // can place anywhere there isn't a station currently
    } else {
        return 0;
    }
}

Actions::Action* Actions::GovernmentGrantConstructor::random_action(){
    // I feel like I shouldn't have written anything while drinking
    int target_city = Map::CITIES[rand() % Map::CITIES.size()].index;
    bool already_station = true;
    // Loop until 4 sure target_city isn't already occupied by a research station
    while(already_station){
        already_station = false; 
        for(Map::City& st: active_board -> get_stations()){
            if(st.index==target_city){
                target_city = Map::CITIES[rand() % Map::CITIES.size()].index;
                already_station=true;
                break;
            }
        }
    }

    for(Players::Player& this_player : active_board -> get_players()){
        for(int e=0;e<this_player.event_cards.size();e++){
            if(this_player.event_cards[e].index==49  && this_player.event_cards[e].name=="Government Grant"){
                if(active_board ->get_stations().size()==6){
                    return new Actions::GovernmentGrant(*active_board,this_player,target_city,rand() % active_board ->get_stations().size());
                }else{
                    return new Actions::GovernmentGrant(*active_board,this_player,target_city);
                }  
            }
        }
    }
    active_board ->broken()=true;
    active_board ->broken_reasons().push_back("[GovernmentGrantConstructor::random_action()] GOVERNMENTGRANT random_action() called but didn't return an action");
}

std::vector<Actions::Action*> Actions::GovernmentGrantConstructor::all_actions(){
    // This monstrosity returns a vector of actions to insert a new research station when:
    //      - It's not in the same position as any existing station

    std::vector<Actions::Action*> full_list;
    for(Players::Player& _using_player: active_board -> get_players()){
        // Find the using player by looking through event cards
        for(int e=0;e<_using_player.event_cards.size();e++){
            if(_using_player.event_cards[e].index==49 && _using_player.event_cards[e].name=="Government Grant"){ // Index of government grant
                if(active_board ->get_stations().size()==6){
                    for(int city=0;city<Map::CITIES.size();city++){
                        bool already_exists = false;
                        for(Map::City& st: active_board->get_stations()){
                            if(city==st.index){
                                already_exists=true;
                                break;
                            }   
                        }
                        if(!already_exists){
                            for(int st=0;st<active_board ->get_stations().size();st++){
                                full_list.push_back(new Actions::GovernmentGrant(*active_board,_using_player,city,st));
                            }  
                        }
                    }
                } else {
                    for(int c=0;c<Map::CITIES.size();c++){
                        bool already_exists=false;
                        for(Map::City& st: active_board->get_stations()){
                            if(c==st.index){
                                already_exists=true;
                                break;
                            } 
                        }
                        if(!already_exists){
                            full_list.push_back(new Actions::GovernmentGrant(*active_board,_using_player,c));
                        }
                    }
                }
            }
        }
    }
    return full_list;
}

bool Actions::GovernmentGrantConstructor::legal(){
    for(Players::Player& pl : active_board ->get_players()){
        for(Decks::PlayerCard e : pl.event_cards){
            if(e.name=="Government Grant" && e.index==49){
                return true;
            }
        }
    }
    return false;
}
// ========================

// ===== QUIETNIGHT =====
Actions::QuietNight::QuietNight(Board::Board& _active_board,Players::Player& _using_player):
    Action(_active_board),
    using_player(_using_player)
    {
    movetype = "QUIETNIGHT";
}

void Actions::QuietNight::execute(){
    using_player.removeCard(Decks::EventCard(48));
    active_board ->quiet_night_status() = true;
}

std::string Actions::QuietNight::repr(){
    return movetype + " used by " + using_player.role.name;
}

Actions::QuietNightConstructor::QuietNightConstructor(Board::Board& _active_board): ActionConstructor(_active_board){}

std::string Actions::QuietNightConstructor::get_movetype(){
    return movetype;
}

int Actions::QuietNightConstructor::n_actions(){
    if(legal()){
        return 1;
    } else {
        return 0;
    }
}

Actions::Action* Actions::QuietNightConstructor::random_action(){
    for(int p=0;p<active_board ->get_players().size();p++){
        Players::Player& this_player = active_board ->get_players()[p];
        for(int e=0;e<this_player.event_cards.size();e++){
            if(this_player.event_cards[e].index==48 && this_player.event_cards[e].name=="Quiet Night"){
                return new Actions::QuietNight(*active_board,this_player);
            }
        }
    }
    active_board ->broken()=true;
    active_board ->broken_reasons().push_back("[QuietNightConstructor::random_action()] QUIETNIGHT random_action() called and didn't return an action");
}

std::vector<Actions::Action*> Actions::QuietNightConstructor::all_actions(){
    std::vector<Actions::Action*> full_list;

    // Find the player that owns the card and return the action with them as the user
    for(int p=0;p<active_board ->get_players().size();p++){
        Players::Player& this_player = active_board ->get_players()[p];
        for(int e=0;e<this_player.event_cards.size();e++){
            if(this_player.event_cards[e].index==48 && this_player.event_cards[e].name=="Quiet Night"){
                full_list.push_back(new Actions::QuietNight(*active_board,this_player));
                return full_list;
            }
        }
    }
    active_board ->broken()=true;
    active_board ->broken_reasons().push_back("[QuietNightConstructor::all_actions()] QUIETNIGHT all_actions() called and didn't return any actions");
}

bool Actions::QuietNightConstructor::legal(){
    // Only bar using this ON the infect step
    if(active_board ->get_turn_action()!=5){
        for(Players::Player& pl : active_board ->get_players()){
            for(Decks::PlayerCard e : pl.event_cards){
                if(e.name=="Quiet Night" && e.index==48){
                    return true;
                }
            }
        }
    }
    return false;
}
// ========================

// ===== EVENT CARD ACTION CONSTRUCTOR =====
// To give a random event card action or list all of the options
// (Depricated - GameLogic will look at each event card constructor individually)
// Actions::EventCardActionConstructor::EventCardActionConstructor(Board::Board& _active_board):ActionConstructor(_active_board){}

// int Actions::EventCardActionConstructor::n_actions(){
//     int n_actions = 0;
//     n_actions+=Actions::AirliftConstructor(*active_board).n_actions();
//     n_actions+=Actions::GovernmentGrantConstructor(*active_board).n_actions();
//     n_actions+=Actions::QuietNightConstructor(*active_board).n_actions();
//     return n_actions;
// }
// ========================

// ===== DONOTHING =====
Actions::DoNothing::DoNothing(Board::Board& _active_board):
    Action(_active_board)
    {
    movetype = "DONOTHING";
}

void Actions::DoNothing::execute(){
    // If used as an "action" during a player turn, increment the counter
    if(active_board ->get_turn_action()<4){
        active_board ->get_turn_action()++;
    }
    // Otherwise this is being called when perhaps using an Event card outside of the turn, and shouldn't change the game state at all.
}

std::string Actions::DoNothing::repr(){
    return movetype;
}

Actions::DoNothingConstructor::DoNothingConstructor(Board::Board& _active_board): ActionConstructor(_active_board){}

std::string Actions::DoNothingConstructor::get_movetype(){
    return movetype;
}

int Actions::DoNothingConstructor::n_actions(){
    if(legal()){
        return 1; // exactly 1 way to do jack
    } else {
        return 0;
    }
}

Actions::Action* Actions::DoNothingConstructor::random_action(){
    return new Actions::DoNothing(*active_board);
}

std::vector<Actions::Action*> Actions::DoNothingConstructor::all_actions(){
    return {new Actions::DoNothing(*active_board)};
}

bool Actions::DoNothingConstructor::legal(){
    if((active_board ->get_turn_action()>=0 && active_board ->get_turn_action()<4) || Actions::QuietNightConstructor(*active_board).legal()){
        // if it's any players turn, OR quiet_night is under consideration (should always be able to not use)
        return true;
    }
    return false;
}
// ========================

// ===== FORCED DISCARD =====
// Can return an action to discard a city card, or use an event card!
Actions::ForcedDiscardAction::ForcedDiscardAction(Board::Board& _active_board,Players::Player& _player_to_discard,Decks::PlayerCard _discard_card):
    Action(_active_board),
    discard_card(_discard_card),
    player_to_discard(_player_to_discard){
        movetype = "FORCEDDISCARD";
}

std::string Actions::ForcedDiscardAction::repr(){
    return movetype+" " + player_to_discard.role.name + " discarded " + discard_card.name;
}

void Actions::ForcedDiscardAction::execute(){
    player_to_discard.removeCard(discard_card);
}

Actions::ForcedDiscardConstructor::ForcedDiscardConstructor(Board::Board& _active_board):
    ActionConstructor(_active_board){};

std::string Actions::ForcedDiscardConstructor::get_movetype(){
    return movetype;
}

int Actions::ForcedDiscardConstructor::n_actions(){
    // Find the first player whose hand is full and build discard actions
    // Unlike other actions, we DON'T care about legality guards here, since it won't ever (and shouldn't) be considered for random selection by an agent.
    for(Players::Player& p: active_board ->get_players()){
        if(p.hand_full()){
            int total_actions = p.hand.size(); // initialize with # of city cards
            for(Decks::PlayerCard e: p.event_cards){
                switch(e.index){
                    case 48:
                        total_actions+=Actions::QuietNightConstructor(*active_board).n_actions();
                        break;
                    case 49:
                        total_actions+=Actions::GovernmentGrantConstructor(*active_board).n_actions();
                        break;
                    case 50:
                        total_actions+=Actions::AirliftConstructor(*active_board).n_actions();
                        break;
                    default:
                        active_board ->broken()=true;
                        active_board ->broken_reasons().push_back("[ForcedDiscardConstructor::n_actions()] Card index ("+std::to_string(e.index)+") isn't an event card");
                        break;
                }
            }
            // Total actions = # of city cards to discard + each achievable action
            return total_actions;
        }
    }
    active_board ->broken()=true;
    active_board ->broken_reasons().push_back("[ForcedDiscardConstructor::n_actions()] FORCEDDISCARD n_actions() called but didn't return anything");
}

Actions::Action* Actions::ForcedDiscardConstructor::random_action(){
    // Distributes probability equally over cards, and THEN uniformly over those card actions if >1
    
    for(Players::Player& p: active_board ->get_players()){
        if(p.hand_full()){
            // pick a card at random
            int card_to_discard_or_use = rand() % p.handsize(); 

            // If the random card is one of the city cards...
            if(card_to_discard_or_use<p.hand.size()){
                // Then just return a forced discard action
                return new Actions::ForcedDiscardAction(*active_board,p,p.hand[card_to_discard_or_use]);
            } else {
                // Otherwise the random card chosen is one of the event cards.
                // In this case use existing constructors to choose a random action to return.
                if(p.event_cards[card_to_discard_or_use - p.hand.size()].index==48){
                    return new Actions::QuietNight(*active_board,p); // Return an action to use quiet night
                } else if(p.event_cards[card_to_discard_or_use - p.hand.size()].index==49){
                    return GovernmentGrantConstructor(*active_board).random_action();
                } else if(p.event_cards[card_to_discard_or_use - p.hand.size()].index==50){
                    return AirliftConstructor(*active_board).random_action();
                }
            }
        }
    }
    active_board ->broken()=true; // should never get here
    active_board ->broken_reasons().push_back("[ForcedDiscardConstructor::random_action()] Forced Discard Constructor has failed to return any action");
}

std::vector<Actions::Action*> Actions::ForcedDiscardConstructor::all_actions(){
    std::vector<Actions::Action*> full_list;
    // First fill the list with the discardable city cards
    for(Players::Player& p: active_board ->get_players()){
        if(p.hand_full()){
            for(Decks::PlayerCard c: p.hand){
                full_list.push_back(new Actions::ForcedDiscardAction(*active_board,p,c));
            }
            for(Decks::PlayerCard e: p.event_cards){
                if(e.index==48 && e.name=="Quiet Night"){
                    std::vector<Actions::Action*> qn_actions = Actions::QuietNightConstructor(*active_board).all_actions();
                    full_list.insert(full_list.end(),qn_actions.begin(),qn_actions.end());
                } else if(e.index==49 && e.name=="Government Grant"){
                    // this is potentially hella wasteful - copying the ~100 ish actions onto a very small vector rather than the other way around
                    // but should only happen once per game so maybe w/e
                    std::vector<Actions::Action*> gg_actions = Actions::GovernmentGrantConstructor(*active_board).all_actions();
                    full_list.insert(full_list.end(),gg_actions.begin(),gg_actions.end());
                } else if(e.index==50 && e.name=="Airlift"){
                    std::vector<Actions::Action*> al_actions = Actions::AirliftConstructor(*active_board).all_actions();
                    full_list.insert(full_list.end(),al_actions.begin(),al_actions.end());
                }   
            }
            return full_list; 
        }
    }
    active_board ->broken()=true;
    active_board ->broken_reasons().push_back("[ForcedDiscardConstructor::all_actions()] FORCEDDISCARD all_actions() called but didn't return anything");
}

bool Actions::ForcedDiscardConstructor::legal(){
    // Should ONLY return legality when there's at least one player with>7 cards in their hand
    // It DOESN't care what phase of the game it is or whose turn it is. This forces an immediate decision node
    for(Players::Player& p: active_board ->get_players()){
        if(p.hand_full()){
            return true;
        }
    }
    return false;
}


