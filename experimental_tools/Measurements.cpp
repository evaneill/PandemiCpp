#include "Measurements.h"

#include "../game_files/Map.h"
#include "../game_files/Players.h"
#include "../game_files/Decks.h"
#include "../game_files/GameLogic.h"
#include "../game_files/Board.h"

#include <chrono>
#include <algorithm>

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

// Upon resetting for a new game, doesn't need to do anything but point to a new board
void Measurements::WinLose::reset(Board::Board& game_board){
    active_board = &game_board;
};

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

// Upon resetting for a new game, doesn't need to do anything but point to a new board
void Measurements::LoseStatus::reset(Board::Board& game_board){
    active_board = &game_board;
};

// ===== GameTreeSize measurements ===== 
Measurements::GameTreeSizeConstructor::GameTreeSizeConstructor(){
    name="Game Tree Variables";
    description="Game Tree attributes including depth (# decisions made by agent) and branching factor (# of decisions at a decision node)";
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
    branching_factors.push_back(GameLogic::Game().n_available_actions(*active_board));
}

// has to account for clearing branching factor record for new game
void Measurements::GameTreeSize::reset(Board::Board& game_board){
    // reset the list of branching factors
    branching_factors.clear();
    // reset the reference to the game board
    active_board = &game_board;
};

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
    if(!quietnightUsed){
        QuietNightUse=-1;
    } else {
        QuietNightUse+=firstQuietNightPresence;
    }
    if(!airliftUsed){
        AirliftUse=-1;
    } else {
        AirliftUse+=firstAirliftPresence;
    }
    if(!governmentgrantUsed){
        GovernmentGrantUse=-1;
    } else {
        GovernmentGrantUse+=firstGovernmentGrantPresence;
    }
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
    bool government_grant_found=false;
    bool airlift_found=false;

    for(Players::Player& p: active_board -> get_players()){
        for(int& card: p.event_cards){
            switch(card){
                case 48:
                    quiet_night_found=true;
                    if(firstQuietNightPresence==-1){
                        firstQuietNightPresence=steps;
                    }
                    break;
                case 49:
                    government_grant_found=true;
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
        quietnightUsed=true;
    };
    if(!government_grant_found && firstGovernmentGrantPresence>=0){
        governmentgrantUsed=true;
    };
    if(!airlift_found && firstAirliftPresence>=0){
        airliftUsed=true;
    };

    if(!quietnightUsed && firstQuietNightPresence>=0){
        QuietNightUse++;
    };
    if(!governmentgrantUsed && firstGovernmentGrantPresence>=0){
        GovernmentGrantUse++;
    };
    if(!airliftUsed && firstAirliftPresence>=0){
        AirliftUse++;
    };
    steps++;
};

// Reset all the tracking variables and reassign board reference
void Measurements::EventCardUse::reset(Board::Board& game_board){
    steps=0;

    firstQuietNightPresence=-1;
    QuietNightUse=-1;
    
    firstAirliftPresence=-1;
    AirliftUse=-1;

    firstGovernmentGrantPresence=-1;
    GovernmentGrantUse=-1;

    quietnightUsed=false;
    airliftUsed=false;
    governmentgrantUsed=false;

    // reset the reference to the game board
    active_board = &game_board;
};

// ===== ActionCount measurements ===== 
Measurements::ActionCountConstructor::ActionCountConstructor(){
    name="Action Counts";
    description="Count of each type of action used during the game. By current construction, *WON'T* be able to capture the last action of the game (whether winning or not)";
}

Measurements::GameMeasurement* Measurements::ActionCountConstructor::construct_measure(Board::Board& active_board){
    return new Measurements::ActionCount(active_board);
}

std::vector<std::string> Measurements::ActionCountConstructor::get_value_keys(){
    return {
        "Move_count",
        "DirectFlight_count",
        "CharterFlight_count",
        "ShuttleFlight_count",
        "OperationsExpertFlight_count",
        "Build_count",
        "Treat_count",
        "Cure_count",
        "Give_count",
        "Take_count",
        "DoNothing_count",
        "Airlift_count",
        "GovernmentGrant_count",
        "QuietNight_count",
        "ForcedDiscard_count"
    };
}

Measurements::ActionCount::ActionCount(Board::Board& _active_board){
    active_board = &_active_board;
}

std::vector<double> Measurements::ActionCount::get_values(){
    return {
        (double) Move_count,
        (double) DirectFlight_count,
        (double) CharterFlight_count,
        (double) ShuttleFlight_count,
        (double) OperationsExpertFlight_count,
        (double) Build_count,
        (double) Treat_count,
        (double) Cure_count,
        (double) Give_count,
        (double) Take_count,
        (double) DoNothing_count,
        (double) Airlift_count,
        (double) GovernmentGrant_count,
        (double) QuietNight_count,
        (double) ForcedDiscard_count
    };
}

//  Unfortunately most of the work here is in updating
void Measurements::ActionCount::update(){
    if(active_board -> LastAction_Move){
        Move_count++;
    }
    if(active_board -> LastAction_DirectFlight){
        DirectFlight_count++;
    }
    if(active_board -> LastAction_CharterFlight){
        CharterFlight_count++;
    }
    if(active_board -> LastAction_ShuttleFlight){
        ShuttleFlight_count++;
    }
    if(active_board -> LastAction_OperationsExpertFlight){
        OperationsExpertFlight_count++;
    }
    if(active_board -> LastAction_Build){
        Build_count++;
    }
    if(active_board -> LastAction_Treat){
        Treat_count++;
    }
    if(active_board -> LastAction_Cure){
        Cure_count++;
    }
    if(active_board -> LastAction_Give){
        Give_count++;
    }
    if(active_board -> LastAction_Take){
        Take_count++;
    }
    if(active_board -> LastAction_DoNothing){
        DoNothing_count++;
    }
    if(active_board -> LastAction_Airlift){
        Airlift_count++;
    }
    if(active_board -> LastAction_GovernmentGrant){
        GovernmentGrant_count++;
    }
    if(active_board -> LastAction_QuietNight){
        QuietNight_count++;
    }
    if(active_board -> LastAction_ForcedDiscard){
        ForcedDiscard_count++;
    }
};

// Reset all the tracking variables and reassign board reference
void Measurements::ActionCount::reset(Board::Board& game_board){
    Move_count = 0;
    DirectFlight_count = 0;
    CharterFlight_count = 0;
    ShuttleFlight_count = 0;
    OperationsExpertFlight_count = 0;
    Build_count = 0;
    Treat_count = 0;
    Cure_count = 0;
    Give_count = 0;
    Take_count = 0;
    DoNothing_count = 0;
    Airlift_count = 0;
    GovernmentGrant_count = 0;
    QuietNight_count = 0;
    ForcedDiscard_count = 0;

    active_board = &game_board;
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
    if(active_board -> is_cured(Map::BLUE) && BlueCured<0){BlueCured = steps;}
    if(active_board -> is_cured(Map::YELLOW) && YellowCured<0){YellowCured = steps;}
    if(active_board -> is_cured(Map::BLACK) && BlackCured<0){BlackCured = steps;}
    if(active_board -> is_cured(Map::RED) && RedCured<0){RedCured = steps;}

    steps++;
};

// Reset all of the tracking variables
void Measurements::CuredDisease::reset(Board::Board& game_board){
    BlueCured = -1;
    YellowCured = -1;
    BlackCured = -1;
    RedCured = -1;

    steps=0;

    active_board = &game_board;
}

// ===== EradicatedDisease measurements ===== 
Measurements::EradicatedDiseaseConstructor::EradicatedDiseaseConstructor(){
    name="Eradicated Status";
    description="Whether each disease is eradicated (<step at which it was eradicated>) or not (<-1>)";
}

Measurements::GameMeasurement* Measurements::EradicatedDiseaseConstructor::construct_measure(Board::Board& active_board){
    return new Measurements::EradicatedDisease(active_board);
}

std::vector<std::string> Measurements::EradicatedDiseaseConstructor::get_value_keys(){
    return {"BlueEradicated","YellowEradicated","BlackEradicated","RedEradicated"};
}

Measurements::EradicatedDisease::EradicatedDisease(Board::Board& _active_board){
    active_board = &_active_board;
}

std::vector<double> Measurements::EradicatedDisease::get_values(){
    return {
        (double) BlueEradicated,
        (double) YellowEradicated,
        (double) BlackEradicated,
        (double) RedEradicated
    };
}

// Update according to any "new" cured diseases.
void Measurements::EradicatedDisease::update(){
    if(active_board -> is_eradicated(Map::BLUE) && BlueEradicated<0){BlueEradicated = steps;}
    if(active_board -> is_eradicated(Map::YELLOW) && YellowEradicated<0){YellowEradicated = steps;}
    if(active_board -> is_eradicated(Map::BLACK) && BlackEradicated<0){BlackEradicated = steps;}
    if(active_board -> is_eradicated(Map::RED) && RedEradicated<0){RedEradicated = steps;}

    steps++;
};

// Reset all of the tracking variables
void Measurements::EradicatedDisease::reset(Board::Board& game_board){
    BlueEradicated = -1;
    YellowEradicated = -1;
    BlackEradicated = -1;
    RedEradicated = -1;

    steps=0;

    active_board = &game_board;
}


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

void Measurements::EpidemicsDrawn::reset(Board::Board& game_board){
    active_board = &game_board;
}

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

void Measurements::ResearchStations::reset(Board::Board& game_board){
    active_board = &game_board;
}

// ===== TimeTaken measurements ===== 
Measurements::TimeTakenConstructor::TimeTakenConstructor(){
    name="Time Duration";
    description="Time (s) from instantiation of the game measure to the request for a measurement value";
}

Measurements::GameMeasurement* Measurements::TimeTakenConstructor::construct_measure(Board::Board& active_board){
    return new Measurements::TimeTaken(active_board);
}

std::vector<std::string> Measurements::TimeTakenConstructor::get_value_keys(){
    return {"TimeTaken"};
}

Measurements::TimeTaken::TimeTaken(Board::Board& _active_board){
    active_board = &_active_board;

    start_time = std::chrono::steady_clock::now();
}

std::vector<double> Measurements::TimeTaken::get_values(){
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start_time;
    return {
        std::chrono::duration<double>(duration).count()
    };
}

// Doesn't need to do anything during update; just read out final time diff
void Measurements::TimeTaken::update(){}

void Measurements::TimeTaken::reset(Board::Board& game_board){
    // reset the start time
    start_time = std::chrono::steady_clock::now();

    active_board = &game_board;   
}