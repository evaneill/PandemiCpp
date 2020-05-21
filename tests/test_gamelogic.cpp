#include <iostream>

#include "../game_files/GameLogic.h"
#include "../game_files/Actions.h"

using namespace std;

int main(){
    srand(time(NULL));
    rand();

    cout << "About to define the game..." << endl;
    GameLogic::Game new_game = GameLogic::Game({0,2,3},4,true);
    cout << "Successfully set up some game logic!" << endl;

    cout << endl << "The reward in the initial state is: " << new_game.reward() << " (should be -10000000)"<< endl;

    cout << endl << "Let's try to get some random (uniformly random) actions..." << endl;
    for(int _=0;_<5;_++){
        Actions::Action* random_action = new_game.get_random_action_uniform();
        cout << "...The action I got is: " << random_action -> repr() << endl;
    }

    cout << endl << "Let's try to get some random (random by group) actions..." << endl;
    for(int _=0;_<5;_++){
        Actions::Action* random_action = new_game.get_random_action_bygroup();
        cout << "...The action I got is: " << random_action -> repr() << endl;
    }

    cout << endl << "Now I'll try to get ALL the actions... (Should contain at least DONOTHING, and MOVE to Miami, MOVE to Chicago, MOVE to Washington)" << endl;
    std::vector<Actions::Action*> possible_actions = new_game.list_actions();
    for(Actions::Action* act_ptr: possible_actions){
        cout << "...action: " << act_ptr -> repr() << endl;
    }
    cout << "I can't actually believe this works..." << endl;
}