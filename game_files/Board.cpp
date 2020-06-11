#include "Board.h"
#include "Decks.h"
#include "Players.h"
#include "Debug.h"

#include <vector>

Board::Board::Board(std::vector<int> roles, int _difficulty){
    for(int r: roles){
        players.push_back(Players::Player(r));
    }

    difficulty = _difficulty;

    player_deck = Decks::PlayerDeck(difficulty);
    infect_deck = Decks::InfectDeck();

    IS_SETUP=false;
}

void Board::Board::setup(bool verbose){
    if(!IS_SETUP){
        int setup_cards;
        switch(players.size()){
            case 2:
                setup_cards=4;
                break;
            case 3:
                setup_cards=3;
                break;
            case 4:
                setup_cards=2;
                break;
            default:
                break;
        }
        // Allot each player their correct number of cards.
        int max_pop = 0, first_player =0, player_idx = 0;
        for(Players::Player& p : players){
            if(verbose){
                DEBUG_MSG(std::endl << "[SETUP] Drawing cards for " << p.role.name << " : ");
            }
            for(int c=0; c<setup_cards;c++){
                Decks::PlayerCard drawn_card = player_deck.draw(true); // true used for setup (i.e. draw from all non-epidemic cards)
                if(verbose){
                    DEBUG_MSG(drawn_card.name << " (color: " << drawn_card.color << ", index: " << drawn_card.index << ", pop: " << drawn_card.population <<") \t");
                }
                if(!drawn_card.event){
                    if(drawn_card.population>max_pop){
                        max_pop = drawn_card.population;
                        first_player = player_idx;
                    }
                }
                p.UpdateHand(drawn_card);
            }
            if(verbose){
                DEBUG_MSG(std::endl << "[SETUP] " << p.role.name << " has cards: ");
                for(Decks::PlayerCard& card: p.hand){
                    DEBUG_MSG(card.name << "; ");
                }
                for(Decks::PlayerCard& card: p.event_cards){
                    DEBUG_MSG(card.name << "; ");
                }
                DEBUG_MSG(std::endl);
            }
            player_idx++;
        }

        // Instantiate some class variables used for drawing hereon out.
        setup_player_deck();

        // The ensuing logic means that, though a random player goes first, the order is not shuffled from that given.
        // This is intended: when people play they'll sit down in a fixed order then learn who goes first.
        // Any randomization in roles should happen above the call to Board().
        if(verbose){
            DEBUG_MSG(std::endl << "[SETUP] First player is in position " << first_player << " (" << players[first_player].role.name << ")" << std::endl); 
        }
        turn = first_player;

        // Make sure all the disease counts in every city are 0
        reset_disease_count();
        
        // Now you have to infect the first 9 cities
        // for each of 1,2, and 3 disease cubes...
        if(verbose){
            DEBUG_MSG(std::endl << "[SETUP] INFECT STEP " << std::endl);
        }
        for(int infect_count = 1 ; infect_count<4;infect_count++){
            // for 3 times
            for(int _ = 0; _<3;_++){
                // infect that city with infect_count # of cubes
                Decks::InfectCard card_to_infect = infect_deck.draw();
                if(verbose){
                    DEBUG_MSG("[SETUP] infecting " << card_to_infect.name << " with " << infect_count << std::endl);
                }
                std::array<int,2> junk = infect_city(card_to_infect.index,card_to_infect.color,infect_count); // ignore the output here
            }
        }
        if(verbose){
            for(int col=0;col<4;col++){
                DEBUG_MSG(std::endl << "[SETUP] =====RESULT OF " << Map::COLORS[col] << " INFECTS =====" << std::endl); 
                for(Map::City& city: Map::CITIES){
                    DEBUG_MSG("[SETUP] " << Map::COLORS[col] << ": " << city.name << " = " << disease_count[col][city.index] << std::endl);
                }
            }
        }

         // Add Atlanta to list of research stations
        AddStation(Map::CITIES[3]); 
        
        // Set a bunch of counters to 0 and statuses to false
        outbreak_count=0;

        turn_actions=0;

        // cards drawn during player draw phase/ infect draw phase
        player_cards_drawn=0;
        infect_cards_drawn=0;

        quiet_night=false;

        // terminal-deciding statuses
        lost=false;
        won=false;
        BROKEN=false;
    } else {
        // This indicates setup() is being called again
        why_it_broke.push_back("setup() called after initial setup()");
        BROKEN = true;
    }
    // set setup to true; won't allow setup() to be called again without breaking
    IS_SETUP = true;
}

void Board::Board::clear(){
    IS_SETUP=false;

    // the players each get reset to atlanta with no hand or previous position
    for(Players::Player& p: players){
        p.reset();
    };

    research_stations.clear();

    player_deck = Decks::PlayerDeck(difficulty);
    infect_deck = Decks::InfectDeck();

    outbreak_count = 0; 
    epidemics_drawn = 0; 

    // have to initialize to 0 on reset
    disease_count[Map::BLUE].fill(0);
    disease_count[Map::YELLOW].fill(0);
    disease_count[Map::BLACK].fill(0);
    disease_count[Map::RED].fill(0);

    // vector of 4 bools: whether or not each disease is cured
    cured= {false,false,false,false};
    // vector of 4 bools: whether or not each disease is eradicated
    eradicated = {false,false,false,false};

    int turn = 0; 

    already_outbroken_cities.clear();

    int turn_actions = 0;

    player_cards_drawn = 0; 
    infect_cards_drawn = 0;

    quiet_night = false;

    lost = false;
    why_lost="";

    won = false;
    BROKEN = false;
    why_it_broke.clear();
}

void Board::Board::is_setup(){
    IS_SETUP=true;
}

void Board::Board::reset_disease_count(){
    // Set all of the disease counts to 0 (I didn't think I had to do this but...)
    for(int col=0;col<4;col++){
        for(Map::City& city: Map::CITIES){
            disease_count[col][city.index]=0;
        }
    }
}

void Board::Board::setup_player_deck(){
    player_deck.setup_shuffle_deck();
}

int Board::Board::remaining_player_cards(){
    return player_deck.remaining_cards();
}

void Board::Board::readd_infect_discard(){
    infect_deck.readd_discard();
}

std::array<int,2> Board::Board::infect(int city_idx, int col,int add){
    std::array<int,2> output = Board::Board::infect_city(city_idx, col, add);
    reset_outbreak_memory();
    return output;
}

std::array<int,2> Board::Board::infect_city(int city_idx, int col,int add){
    // count outbreaks the occur as a result of this logic
    int n_outbreaks = 0;

    // count blocked outbreaks accrued during the execution of this logic
    int blocked_outbreaks = 0;

    // Check for existence & adjacency of quarantine specialist, if the game has already been set up
    if(IS_SETUP){
        for(Players::Player& p: players){
            if(p.role.quarantinespecialist){
                // If they're in the same city
                if(p.get_position().index==city_idx){
                    if(disease_count[col][city_idx]+add>3){
                        return {0,1};
                    } else {
                        return {0,0};
                    }
                }
                // Or if they're in an adjacent city
                for(int n: p.get_position().neighbors){
                    if(city_idx==n){
                        if(disease_count[col][city_idx]+add>3){
                            return {0,1};
                        } else {
                            return {0,0};
                        }
                    }
                }
                break;
            }
        }
    }
    for(int c: already_outbroken_cities){
        if(c==city_idx){
            return {0,0};
        }
    }
    // To get to this point there can't have been a blocking quarantine specialist
    // This city also can't have been a previously outbroken city during the resolution of this logic
    // If adding the suggested amount would result in there being >3 cubes...
    if((disease_count[col][city_idx]+add)>3){
        // set the count there to 3 whether or not it was already
        disease_count[col][city_idx] = 3;

        // outbreak this city and color
        return outbreak(city_idx,col);
    } else{
        // Otherwise just add "add" number of cubes to it if this city isn't outbroken
        disease_count[col][city_idx]+=add;
        return {0,0};
    }
}

// This outbreak resolution ignores the fact that players get to CHOOSE the resolution order of outbreaks, potentially to their great advantage.
// Ignoring player preference literally can be the difference of a few outbreaks, but won't include that here.
// _Very_ rare that order matters in outbreak resolution. Usually at most 2.
// Could optimize resolution in favor of the player by always outbreaking the neighbors with 3 cubes FIRST, then the remainder
// Not even sure that that would be best. Regardless it's irrelevant in almost all cases.
std::array<int,2> Board::Board::outbreak(int city_idx,int col){
    // increment game outbreak counter
    outbreak_count++;
    // immediate fail - no need to keep evolving the state at this point
    if(outbreak_count>7){
        lost=true;
        return {1,0};
    }

    // track this-resolution outbreaks and blocked outbreaks.
    int n_outbreaks =1, n_blocked = 0;

    // Add this city being outbroken to the list of those outbroken on this resolution
    already_outbroken_cities.push_back(city_idx);

    for(int neighbor: Map::CITIES[city_idx].neighbors){
        std::array<int,2> infect_result= infect_city(neighbor,col,1);
        if(infect_result[0]>0 || infect_result[1]>0){
            // If an outbreak event did or would have happened at the neighbor, then it's accounted for
            already_outbroken_cities.push_back(neighbor);
        }
        n_outbreaks+=infect_result[0];
        n_blocked+=infect_result[1];
    }
    return {n_outbreaks,n_blocked};
}

std::array<int,2> Board::Board::outbreak(Map::City& city, int col){
    return outbreak(city.index,col);
}

void Board::Board::reset_outbreak_memory(){
    already_outbroken_cities.clear();
}

bool Board::Board::is_terminal(){
    updatestatus();
    if(won || lost){
        return true;
    } else if(BROKEN){
        return true;
    } else {
        return false;
    }
};

int Board::Board::win_lose(){
    if(won){
        return 1;
    } else if(lost){
        return 0;
    } else {
        // Right now this is actually only called by `Measurement`s
        // Won't force the board to break just because a reward was requested at a bad time
        return -100000;
    }
}

void Board::Board::update_medic_position(){
    for(Players::Player& p: players){
        // If any player is the medic
        if(p.role.medic){
            for(int col=0;col<4;col++){
                // and a disease is cured
                if(cured[col]){
                    // disease count of that color on their position should be 0
                    disease_count[col][p.get_position().index]=0;
                }
            }
        }
    }
}

void Board::Board::update_eradicated_status(){
    for(int col=0;col<4;col++){
        // If the disease has a total of 0 cubes on the board and is cured...
        if(std::accumulate(disease_count[col].begin(),disease_count[col].end(),0)==0 && cured[col]){
            // then it's eradicated!
            eradicated[col]=true;
        }
    }
}
// The goal of this function is to check win/lose status. Broken status is handled by game logic above.
// This should be partially made redundant by logic built into actions that can induce a win/loss
// But redundancy for important things is good!
void Board::Board::updatestatus(){
    // First have to remove all cubes of cured diseases from position of medic, if existent
    // It ONLY makes sense to put this here because I know that this will get called after ever agent AND stochastic action.
    update_medic_position();
    update_eradicated_status();

    if(std::accumulate(cured.begin(),cured.end(),0)==4){
        // the only way to win is to have 4 cured diseases
        won = true;
    }
    if(player_deck.isempty()){
        // If there are no more player cards, you lost.
        why_lost = "Ran out of player cards!";
        lost = true;
    }
    if(outbreak_count>7){
        // If there are >7 outbreaks
        why_lost =  "Found >7 outbreaks!";
        lost=true;
    }
    if(!disease_count_safe()){
        // If any disease has >24 cubes on the board
        why_lost = "Found that disease cube count wasn't safe!";
        lost = true;
    }
}

Players::Player& Board::Board::active_player(){
    return players[turn];
}

std::vector<Map::City*>& Board::Board::get_stations(){
    return research_stations;
}

void Board::Board::AddStation(Map::City& new_station){
    research_stations.push_back(&new_station);
}

void Board::Board::RemoveStation(int station_city_idx){
    // Takes the city index of the station to be erased
    for(int st=0; st<research_stations.size();st++){
        if(research_stations[st] -> index==station_city_idx){
            research_stations.erase(research_stations.begin() + st);
            return;
        }
    }
}

std::array<std::array<int,48>,4>& Board::Board::get_disease_count(){
    return disease_count;
}

bool  Board::Board::is_eradicated(int col){
    return eradicated[col];
}

void Board::Board::Eradicate(int col){
    eradicated[col]=true;
}

bool Board::Board::is_cured(int col){
    return cured[col];
}

void Board::Board::Cure(int col){
    cured[col]=true;
}

std::vector<Players::Player>& Board::Board::get_players(){
    return players;
}

bool& Board::Board::quiet_night_status(){
    return quiet_night;
}

bool& Board::Board::has_won(){
    return won;
}

bool& Board::Board::has_lost(){
    return lost;
}

int& Board::Board::get_turn_action(){
    return turn_actions;
}

Decks::PlayerCard Board::Board::draw_playerdeck_inplace(){
    return player_deck.draw_inplace();
}

int& Board::Board::get_player_cards_drawn(){
    return player_cards_drawn;
}

void Board::Board::updatePlayerDeck(Decks::PlayerCard card){
    player_deck.update(card);
}

int& Board::Board::get_infect_cards_drawn(){
    return infect_cards_drawn;
}

int Board::Board::n_infect_cards(bool top){
    return infect_deck.top_group_size(top);
}

Decks::InfectCard Board::Board::draw_infectdeck_bottom_inplace(){
    return infect_deck.draw_bottom_inplace();
}

Decks::InfectCard Board::Board::draw_infectdeck_inplace(){
    return infect_deck.draw_inplace();
}

void Board::Board::updateInfectDeck(Decks::InfectCard card,bool bottom){
    infect_deck.update(card,bottom);
}

int& Board::Board::get_outbreak_count(){
    return outbreak_count;
}

int Board::Board::get_epidemic_count(){
    return player_deck._epidemics_drawn();
}

bool Board::Board::epidemic_possible(){
    return player_deck.epidemic_possible();
}

int& Board::Board::get_difficulty(){
    return difficulty;
}

int Board::Board::get_infection_rate(){
    return INFECTION_COUNTER[player_deck._epidemics_drawn()];
}

int& Board::Board::get_turn(){
    return turn;
}

int Board::Board::disease_sum(int col){
    int sum=0;
    for(int city;city<Map::CITIES.size();city++){
        sum+=disease_count[col][city];
    }
    return sum;
}

bool Board::Board::disease_count_safe(){
    if(disease_sum(Map::BLUE)>24 || disease_sum(Map::YELLOW)>24 || disease_sum(Map::BLACK)>24 || disease_sum(Map::RED)>24){
        return false;
    }
    return true;
}

bool Board::Board::player_deck_nonempty(){
    return !player_deck.isempty();
}

bool Board::Board::outbreak_count_safe(){
    return outbreak_count<8;
}

bool& Board::Board::broken(){
    return BROKEN;
}

std::vector<std::string>& Board::Board::broken_reasons(){
    return why_it_broke;
}

std::string&  Board::Board::get_lost_reason(){
    return why_lost;
}