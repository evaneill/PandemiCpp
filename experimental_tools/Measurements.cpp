#include "Measurements.h"

#include "../game_files/Map.h"
#include "../game_files/Players.h"
#include "../game_files/Decks.h"
#include "../game_files/GameLogic.h"

Measurements::Measurement::Measurement(Board::Board& _active_board){
    active_board = &_active_board;
}

// ===== Win-Lose measurements ===== 
Measurements::WinLose::WinLose(Board::Board& _active_board):
    Measurement(_active_board){
        name="Win-Lose";
        description="Final game reward";
}

std::vector<double> Measurements::WinLose::get_values(){
    return {(double) active_board -> win_lose()};
}

std::vector<std::string> Measurements::WinLose::get_value_keys(){
    return {"GameWon"};
}
// Doesn't need to do any updating
void Measurements::WinLose::update(){}; 

// ===== LoseStatus measurements ===== 
Measurements::LoseStatus::LoseStatus(Board::Board& _active_board):
    Measurement(_active_board){
        name="Loss Status Variables";
        description="Values of potentially loss-inducing board attributes";
}

std::vector<double> Measurements::LoseStatus::get_values(){
    return {
        (double) active_board -> get_outbreak_count(),
        (double) active_board -> remaining_player_cards(),
        (double) (*active_board).disease_sum(Map::BLUE),
        (double) (*active_board).disease_sum(Map::YELLOW),
        (double) (*active_board).disease_sum(Map::BLACK),
        (double) (*active_board).disease_sum(Map::RED)
    };
}

std::vector<std::string> Measurements::LoseStatus::get_value_keys(){
    return {"OutbreakCount","PlayerCardsLeft","BlueDiseasePresent","YellowDiseasePresent","BlackDiseasePresent","RedDiseasePresent"};
}

// Doesn't need to do anything during update; just read out final status
void Measurements::LoseStatus::update(){};

// ===== GameTreeSize measurements ===== 
Measurements::GameTreeSize::GameTreeSize(Board::Board& _active_board):
    Measurement(_active_board){
        name="Game Tree Variables";
        description="Game Tree attributes including depth and branching factor";
}

std::vector<double> Measurements::GameTreeSize::get_values(){
    int sum=0;
    for(int& val: branching_factors){
        sum+=val;
    }
    double average = ((double) sum)/((double) branching_factors.size());
    // reset sum to calculate standard deviation
    double variance=0;
    for(int& val: branching_factors){
        variance+=std::pow(val-average,2);
    }
    
    return {
        (double) branching_factors.size(),
        (double) *std::min_element(branching_factors.begin(),branching_factors.end()),
        (double) *std::max_element(branching_factors.begin(),branching_factors.end()),
        average,
        std::sqrt(variance/((double) branching_factors.size())) // sqrt of variance = sqrt of ([1/N]SUM(x_i-u)^2)
    };
}

std::vector<std::string> Measurements::GameTreeSize::get_value_keys(){
    return {"Depth","MinBranch","MaxBranch","AvgBranch","StdDevBranch"};
}

// Doesn't need to do much during update; just push on the current branching factor
void Measurements::GameTreeSize::update(){
    // I feel like this is a little bit jank
    branching_factors.push_back(GameLogic::Game(*active_board).n_available_actions());
}

// ===== EventCardUse measurements ===== 
Measurements::EventCardUse::EventCardUse(Board::Board& _active_board):
    Measurement(_active_board){
        name="Event Card Usage";
        description="Values to capture the usage of event cards by the agent";
}

std::vector<double> Measurements::EventCardUse::get_values(){
    return {
        (double) firstQuietNightPresence,
        (double) QuietNightUse,
        (double) firstAirliftPresence,
        (double) AirliftUse,
        (double) firstGovernmentGrantPresence,
        (double) GovernmentGrantUse
    };
}

std::vector<std::string> Measurements::EventCardUse::get_value_keys(){
    return {"firstQuietNightPresence","QuietNightUse","firstAirliftPresence","AirliftUse","firstGovernmentGrantPresence","GovernmentGrantUse"};
}

//  Unfortunately most of the work here is in updating
void Measurements::EventCardUse::update(){
    bool quiet_night_found=false;
    bool government_grand_found=false;
    bool airlift_found=false;
    for(Players::Player& p: active_board -> get_players()){
        for(Decks::PlayerCard& card: p.event_cards){
            switch(card.index){
                case 48:
                    quiet_night_found=true;
                    if(firstQuietNightPresence==-1){
                        firstQuietNightPresence=steps;
                    }
                    break;
                case 49:
                    government_grand_found=true;
                    if(firstGovernmentGrantPresence==-1){
                        firstGovernmentGrantPresence=steps;
                    }
                    break;
                case 50:
                    airlift_found=true;
                    if(firstAirliftPresence==-1){
                        firstAirliftPresence=steps;
                    }
                    break;
            }
        }
    }
    if(!quiet_night_found && firstQuietNightPresence>=0){
        QuietNightUse=steps;
    };
    if(!government_grand_found && firstGovernmentGrantPresence>=0){
        GovernmentGrantUse=steps;
    };
    if(!airlift_found && firstAirliftPresence>=0){
        AirliftUse=steps;
    };
    steps++;
};

// ===== LoseStatus measurements ===== 
Measurements::CuredDisease::CuredDisease(Board::Board& _active_board):
    Measurement(_active_board){
        name="Cure Status";
        description="Whether each disease is cured (1) or not (0)";
}

std::vector<double> Measurements::CuredDisease::get_values(){
    return {
        // I _think_ booleans get converted to 0/1 on casting to double
        (double) active_board -> get_cured()[Map::BLUE],
        (double) active_board -> get_cured()[Map::YELLOW],
        (double) active_board -> get_cured()[Map::BLACK],
        (double) active_board -> get_cured()[Map::RED],
    };
}

std::vector<std::string> Measurements::CuredDisease::get_value_keys(){
    return {"BlueCured","YellowCured","BlackCured","RedCured"};
}

// Doesn't need to do anything during update; just read out final status
void Measurements::CuredDisease::update(){};

// ===== EpidemicsDrawn measurements ===== 
Measurements::EpidemicsDrawn::EpidemicsDrawn(Board::Board& _active_board):
    Measurement(_active_board){
        name="Epidemics Drawn";
        description="How many epidemic cards were drawn during the game";
}

std::vector<double> Measurements::CuredDisease::get_values(){
    return {
        (double) active_board -> get_epidemic_count()
    };
}

std::vector<std::string> Measurements::CuredDisease::get_value_keys(){
    return {"EpidemicsDrawn"};
}

// Doesn't need to do anything during update; just read out final status
void Measurements::CuredDisease::update(){};
