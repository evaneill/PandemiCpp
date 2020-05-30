#include "Measurements.h"

#include "../game_files/Map.h"
#include "../game_files/Players.h"
#include "../game_files/Decks.h"
#include "../game_files/GameLogic.h"
#include "../game_files/Board.h"

Measurements::GameMeasurement::GameMeasurement(){}

Measurements::MeasurementConstructor::MeasurementConstructor(){}

// ===== Win-Lose measurements ===== 
Measurements::WinLoseConstructor::WinLoseConstructor(){
    name = "Win-Lose";
    description = "Whether the agent won (1) or lost (0)";
}
Measurements::GameMeasurement* Measurements::WinLoseConstructor::construct_measure(Board::Board& active_board){
    return new Measurements::WinLose(active_board);
}

std::vector<std::string> Measurements::WinLoseConstructor::get_value_keys(){
    return {"GameWon"};
}

Measurements::WinLose::WinLose(Board::Board& _active_board){
    active_board = &_active_board;
}

std::vector<double> Measurements::WinLose::get_values(){
    return {(double) active_board -> win_lose()};
}

// Doesn't need to do any updating
void Measurements::WinLose::update(){}; 

// ===== LoseStatus measurements ===== 
Measurements::LoseStatusConstructor::LoseStatusConstructor(){
    name="Loss Status Variables";
    description="Values of potentially loss-inducing board attributes";
}

Measurements::GameMeasurement* Measurements::LoseStatusConstructor::construct_measure(Board::Board& active_board){
    return new Measurements::LoseStatus(active_board);
}

std::vector<std::string> Measurements::LoseStatusConstructor::get_value_keys(){
    return {"OutbreakCount","PlayerCardsLeft","BlueDiseasePresent","YellowDiseasePresent","BlackDiseasePresent","RedDiseasePresent"};
}

Measurements::LoseStatus::LoseStatus(Board::Board& _active_board){
    active_board = &_active_board;
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

// Doesn't need to do anything during update; just read out final status
void Measurements::LoseStatus::update(){};

// ===== GameTreeSize measurements ===== 
Measurements::GameTreeSizeConstructor::GameTreeSizeConstructor(){
    name="Game Tree Variables";
    description="Game Tree attributes including depth and branching factor";
}

Measurements::GameMeasurement* Measurements::GameTreeSizeConstructor::construct_measure(Board::Board& active_board){
    return new Measurements::GameTreeSize(active_board);
}

std::vector<std::string> Measurements::GameTreeSizeConstructor::get_value_keys(){
    return {"Depth","MinBranch","MaxBranch","AvgBranch","StdDevBranch"};
}

Measurements::GameTreeSize::GameTreeSize(Board::Board& _active_board){
    active_board = &_active_board;
}

std::vector<double> Measurements::GameTreeSize::get_values(){
    int sum=0;
    for(int& val: branching_factors){
        sum+=val;
    }
    double average = ((double) sum)/((double) branching_factors.size());

    double variance=0;
    for(int& val: branching_factors){
        variance+=std::pow(val-average,2);
    }
    
    if(branching_factors.size()>0){
         return {
            (double) branching_factors.size(),
            (double) *std::min_element(branching_factors.begin(),branching_factors.end()),
            (double) *std::max_element(branching_factors.begin(),branching_factors.end()),
            average,
            std::sqrt(variance/((double) branching_factors.size())) // sqrt of variance = sqrt of ([1/N]SUM(x_i-u)^2)
        };
    } else {
         return {
            (double) branching_factors.size(),
            (double) 0,
            (double) 0,
            (double) 0,
            (double) 0
        };
    }
   
}

// Doesn't need to do much during update; just push on the current branching factor
void Measurements::GameTreeSize::update(){
    // I feel like this is a little bit jank
    branching_factors.push_back(GameLogic::Game(*active_board).n_available_actions());
}

// ===== EventCardUse measurements ===== 
Measurements::EventCardUseConstructor::EventCardUseConstructor(){
    name="Event Card Usage";
    description="Values to capture the usage of event cards by the agent";
}

Measurements::GameMeasurement* Measurements::EventCardUseConstructor::construct_measure(Board::Board& active_board){
    return new Measurements::EventCardUse(active_board);
}

std::vector<std::string> Measurements::EventCardUseConstructor::get_value_keys(){
    return {"firstQuietNightPresence","QuietNightUse","firstAirliftPresence","AirliftUse","firstGovernmentGrantPresence","GovernmentGrantUse"};
}

Measurements::EventCardUse::EventCardUse(Board::Board& _active_board){
    active_board = &_active_board;
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

// ===== CureDisease measurements ===== 
Measurements::CuredDiseaseConstructor::CuredDiseaseConstructor(){
    name="Cure Status";
    description="Whether each disease is cured (<step at which it was cured>) or not (<-1>)";
}

Measurements::GameMeasurement* Measurements::CuredDiseaseConstructor::construct_measure(Board::Board& active_board){
    return new Measurements::CuredDisease(active_board);
}

std::vector<std::string> Measurements::CuredDiseaseConstructor::get_value_keys(){
    return {"BlueCured","YellowCured","BlackCured","RedCured"};
}

Measurements::CuredDisease::CuredDisease(Board::Board& _active_board){
    active_board = &_active_board;
}

std::vector<double> Measurements::CuredDisease::get_values(){
    return {
        (double) BlueCured,
        (double) YellowCured,
        (double) BlackCured,
        (double) RedCured
    };
}

// Update according to any "new" cured diseases.
void Measurements::CuredDisease::update(){
    if(active_board -> get_cured()[Map::BLUE] && BlueCured<0){BlueCured = steps;}
    if(active_board -> get_cured()[Map::YELLOW] && YellowCured<0){YellowCured = steps;}
    if(active_board -> get_cured()[Map::BLACK] && BlackCured<0){BlackCured = steps;}
    if(active_board -> get_cured()[Map::RED] && RedCured<0){RedCured = steps;}

    steps++;
};

// ===== EpidemicsDrawn measurements ===== 
Measurements::EpidemicsDrawnConstructor::EpidemicsDrawnConstructor(){
    name="Epidemics Drawn";
    description="How many epidemic cards were drawn during the game";
}

Measurements::GameMeasurement* Measurements::EpidemicsDrawnConstructor::construct_measure(Board::Board& active_board){
    return new Measurements::EpidemicsDrawn(active_board);
}

std::vector<std::string> Measurements::EpidemicsDrawnConstructor::get_value_keys(){
    return {"EpidemicsDrawn"};
}

Measurements::EpidemicsDrawn::EpidemicsDrawn(Board::Board& _active_board){
    active_board = &_active_board;
}

std::vector<double> Measurements::EpidemicsDrawn::get_values(){
    return {
        (double) active_board -> get_epidemic_count()
    };
}

// Doesn't need to do anything during update; just read out final status
void Measurements::EpidemicsDrawn::update(){}

// ===== ResearchStations measurements ===== 
Measurements::ResearchStationsConstructor::ResearchStationsConstructor(){
    name="Number of Research Stations";
    description="How many research stations are on the board at game end";
}

Measurements::GameMeasurement* Measurements::ResearchStationsConstructor::construct_measure(Board::Board& active_board){
    return new Measurements::ResearchStations(active_board);
}

std::vector<std::string> Measurements::ResearchStationsConstructor::get_value_keys(){
    return {"ResearchStations"};
}

Measurements::ResearchStations::ResearchStations(Board::Board& _active_board){
    active_board = &_active_board;
}

std::vector<double> Measurements::ResearchStations::get_values(){
    return {
        (double) active_board -> get_stations().size()
    };
}

// Doesn't need to do anything during update; just read out final status
void Measurements::ResearchStations::update(){}