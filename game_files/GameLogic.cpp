#include "GameLogic.h"
#include "Board.h"
#include "SanityCheck.h"

GameLogic::Game::Game(std::vector<int> roles, int difficulty): 
    active_board(Board::Board(roles,difficulty)),

    // Link the active board to each constructor
    MoveCon(Actions::MoveConstructor(active_board)),
    DirectFlightCon(Actions::DirectFlightConstructor(active_board)),
    CharterFlightCon(Actions::CharterFlightConstructor(active_board)),
    ShuttleFlightCon(Actions::ShuttleFlightConstructor(active_board)),
    OperationsExpertFlightCon(Actions::OperationsExpertFlightConstructor(active_board)),
    BuildCon(Actions::BuildConstructor(active_board)),
    TreatCon(Actions::TreatConstructor(active_board)),
    CureCon(Actions::CureConstructor(active_board)),
    GiveCon(Actions::GiveConstructor(active_board)),
    TakeCon(Actions::TakeConstructor(active_board)),
    DoNothingCon(Actions::DoNothingConstructor(active_board)),

    // Do the same with the forced discard constructor
    ForcedDiscardCon(Actions::ForcedDiscardConstructor(active_board)),

    // Do the same with event card action constructors
    AirliftCon(Actions::AirliftConstructor(active_board)),
    GovernmentGrantCon(Actions::GovernmentGrantConstructor(active_board)),
    QuietNightCon(Actions::QuietNightConstructor(active_board)),
    
    // Instantiate the list of pointers to the constructors
    PlayerConstructorList({
            &MoveCon,
            &DirectFlightCon,
            &CharterFlightCon,
            &ShuttleFlightCon,
            &OperationsExpertFlightCon,
            &BuildCon,
            &TreatCon,
            &CureCon,
            &GiveCon,
            &TakeCon,
            &DoNothingCon,
            &AirliftCon,
            &GovernmentGrantCon,
            &QuietNightCon}),

    // Instantiate the stochastic action constructor
    StochasticCon(StochasticActions::StochasticActionConstructor(active_board))
    {
        active_board.setup();
}

Actions::Action GameLogic::Game::get_random_action_uniform(){
    // Goal is to get a UNIFORM distribution over all legal actions
    // Will _try_ to speed this up over just generating all actions by:
    //      counting player actions first
    //      taking a uniform int in [0,sum of total actions-1]
    //      going through the same action options in order until this falls within one of the ranges

    int randomized = rand() % n_available_actions();

    // Under this scheme actions should appear uniformly
    for(Actions::ActionConstructor* con_ptr: PlayerConstructorList){
        if(randomized < (con_ptr -> n_actions())){
            return (con_ptr -> random_action());
        }else{
            randomized - (con_ptr -> n_actions());
        }
    }
}

Actions::Action GameLogic::Game::get_random_action_bygroup(){
    // Goal is to choose FIRST uniformly over _types_ of actions
    // Then choose uniformly among legal actions of that type

    int randomized = rand() % PlayerConstructorList.size();

    return PlayerConstructorList[randomized] -> random_action();
}

int GameLogic::Game::n_available_actions(){
    int total_actions=0;
    for(Actions::ActionConstructor* con_ptr: PlayerConstructorList){
        total_actions+=(con_ptr -> n_actions());
    }

    return  total_actions;
}

std::vector<Actions::Action> GameLogic::Game::list_actions(){
    // This represents a lazy execution of stochastic actions - waits until the next time any action is asked for before resolving any stochasticity/discard
    if(ForcedDiscardCon.legal()){
        // ALWAYS return a choice of discard actions if it's possible.
        // Includes possible plays of event cards
        return ForcedDiscardCon.all_actions();
    }
    while(StochasticCon.legal()){
        Actions::Action next_action = StochasticCon.get_action();
        next_action.execute();
        // Check for need to discard after each execution
        if(ForcedDiscardCon.legal()){
            return ForcedDiscardCon.all_actions();
        }
    }
    // If the logic has reached this point, then we're just letting the player (agent) play. Return action options.
    std::vector<Actions::Action> full_out;

    for(Actions::ActionConstructor* con_ptr: PlayerConstructorList){
        if(con_ptr -> legal()){
            std::vector<Actions::Action> these_actions = (con_ptr -> all_actions());
            full_out.insert(full_out.end(),these_actions.begin(),these_actions.end());
        }
    }
    return full_out;

}

bool GameLogic::Game::is_terminal(bool sanity_check=false){
    // sanity_check does some logical checking of the game state to enforce brokenness
    // costs some computation, which is why it's optional.
    if(sanity_check){
        SanityCheck::CheckBoard(active_board);
    }
    return active_board.is_terminal();
}

int GameLogic::Game::reward(){
    if(active_board.has_lost()){
        return 0;
    } else if(active_board.has_won()){
        return 1;
    } else {
        return NULL;
    }
}