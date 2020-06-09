#include <iostream>
#include <vector>
#include <string>
#include <unordered_set>
#include <ctime>

#include "../game_files/Map.h"
#include "../game_files/Decks.h"
#include "../game_files/StochasticActions.h"
#include "../game_files/Actions.h"
#include "../game_files/Board.h"

// void print(std::set<int> const &s)
// {
//     std::copy(s.begin(),
//             s.end(),
//             std::ostream_iterator<int>(std::cout, " "));
// }
using namespace std;

int main(){
    std::srand(time(NULL));
    int junk = rand(); // For some reason, first rand is ALWAYS seed*constant. This biases the first choice to a value that almost guarantees epidemic when seeded just before that choice.


    cout << "Get the city map...\n";
    std::vector<Map::City>& cities = Map::CITIES;
    cout << "... success! The map has " << cities.size() << " cities (Should be 48)\n\n";

    cout << "Instantiate a board with verbose setup..." << endl;
    Board::Board new_board = Board::Board({0,2,3},4);
    new_board.setup(true);
    cout << "\nAfter pretending to setup the game, there are " << new_board.remaining_player_cards() << " non epidemic cards left to setup. (Should be 55 - 3*3)\n\n";


    StochasticActions::PlayerDeckDrawActionConstructor player_draw_constructor = StochasticActions::PlayerDeckDrawActionConstructor();

    cout << "\n======= Now we want to make sure that the draw() action acts as it should.\n";
    cout << "======= I will draw through the entirety of the FIRST chunk in sequence:\n";
    for(int i=1;i<12+1;i++){
        Decks::PlayerCard card = new_board.draw_playerdeck_inplace();
        cout << "\tCard " << i << " is " << card.name;
        new_board.updatePlayerDeck(card);
        cout << " \t\t (" << new_board.remaining_player_cards() << " player cards left to draw)\n";
        if(card.epidemic==true){
            cout << "\t --- > This should be the only epidemic drawn in this chunk!!!! < -----\n";
        }
    }

    cout << "======= I will draw through the entirety of the SECOND chunk in sequence:\n";
    for(int i=1;i<12+1;i++){
        Decks::PlayerCard card = new_board.draw_playerdeck_inplace();
        cout << "\tCard " << i+12 << " is " << card.name;
        new_board.updatePlayerDeck(card);
        cout << " \t\t (" << new_board.remaining_player_cards() << " player cards left to draw)\n";
        if(card.epidemic==true){
            cout << "\t --- > This should be the only epidemic drawn in this chunk!!!! < -----\n";
        }
    }

    cout << "======= I will draw through the entirety of the THIRD chunk in sequence:\n";
    for(int i=1;i<11+1;i++){
        Decks::PlayerCard card = new_board.draw_playerdeck_inplace();
        cout << "\tCard " << i+23 << " is " << card.name;
        new_board.updatePlayerDeck(card);
        cout << " \t\t (" << new_board.remaining_player_cards() << " player cards left to draw)\n";
        if(card.epidemic==true){
            cout << "\t --- > This should be the only epidemic drawn in this chunk!!!! < -----\n";
        }
    }

    cout << "======= I will draw through the entirety of the FOURTH chunk in sequence:\n";
    for(int i=1;i<11+1;i++){
        Decks::PlayerCard card = new_board.draw_playerdeck_inplace();
        cout << "\tCard " << i+34 << " is " << card.name;
        new_board.updatePlayerDeck(card);
        cout << " \t\t (" << new_board.remaining_player_cards() << " player cards left to draw)\n";
        if(card.epidemic==true){
            cout << "\t --- > This should be the only epidemic drawn in this chunk!!!! < -----\n";
        }
    }
}