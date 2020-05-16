#include <iostream>
#include <vector>
#include <string>
#include <unordered_set>
#include <ctime>

#include "../game_files/Map.h"
#include "../game_files/Decks.h"

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


    cout << "Instantiate a city map...\n";
    Map::Cities cities = Map::Cities();
    cout << "... success!\n\n";

    cout << "======= Instantiate a PlayerDeck w/ difficulty 4...\n";
    Decks::PlayerDeck my_deck = Decks::PlayerDeck(4, cities);
    cout << "... success!\n";
    cout << "This instantiated deck has " << my_deck._remaining_nonepi_cards() << " non-epidemic cards total. (Should be 51)\n\n";

    cout << "======= Pretend like we're setting up with 3 players (3 cards per player)...\n";
    vector<Decks::PlayerCard> player_hand (3);
    for(int p=0;p<3;p++){
        cout << "======= Drawing cards for player " << p+1 << "...\n";
        for(int l=0;l<3;l++){
            player_hand[l]=my_deck.draw(true);
        }
        cout << "Player " << p+1 << " drew " << player_hand.size() << " cards (should be 3).\n";
        cout << "There are now " << my_deck._remaining_nonepi_cards() << " non-epidemic cards left (should be 51 - 3*" << p+1 << ").\n";

        for(int c=0;c<player_hand.size();c++){
            cout << "--- Card " << c+1 << ":  " << player_hand[c].index << " (should be 0<= card.index <= 50), which is " << player_hand[c].name << "\n";
        }
    }
    cout << "\nAfter pretending to setup the game, there are " << my_deck._remaining_nonepi_cards() << " non epidemic cards left to setup. (Should be 51 - 3*3)\n\n";

    cout << "======= Using these remaining cards, we organize it into 4 chunks and shuffle them...\n";
    my_deck.setup_shuffle_deck();
    cout << "After doing this, we have:\n";
    cout << " --- " << my_deck._remainder() << " fat chunks of size " << my_deck._fat_chunk_size() << " (should be 2 of them)\n";
    cout << " --- " << 4 - my_deck._remainder() << " regular chunks of size " << my_deck._chunk_size() << " (should be 2 of them)\n";
    cout << " --- " << my_deck._total_cards_drawn() << " cards drawn during play so far. (Should be 0)\n";
    cout << " --- " << my_deck._drawn_cards() << " cards now existent outside of the deck on the board. (Should be 9)\n";

    cout << "\n======= Now we want to make sure that the draw() action acts as it should.\n";
    cout << "======= I will draw through the entirety of the FIRST chunk in sequence:\n";
    for(int i=1;i<my_deck._fat_chunk_size()+1;i++){
        Decks::PlayerCard my_card = my_deck.draw();
        cout << "\tCard " << i << " is " << my_card.name;
        cout << " \t\t (" << my_deck._total_cards_drawn() << " total player cards drawn during play now.)";
        cout << " \t\t (" << my_deck._remaining_nonepi_cards()+4 -my_deck._epidemics_drawn() << " player cards left to draw)\n";
        if(my_card.epidemic==true){
            cout << "\t --- > This should be the only epidemic drawn in this chunk!!!! < -----\n";
        }
    }

    cout << "======= I will draw through the entirety of the SECOND chunk in sequence:\n";
    for(int i=1;i<my_deck._fat_chunk_size()+1;i++){
        Decks::PlayerCard my_card = my_deck.draw();
        cout << "\tCard " << i+my_deck._fat_chunk_size() << " is " << my_card.name;
        cout << " \t\t (" << my_deck._total_cards_drawn() << " total player cards drawn during play now.)";
        cout << " \t\t (" << my_deck._remaining_nonepi_cards()+4 -my_deck._epidemics_drawn() << " player cards left to draw)\n";
        if(my_card.epidemic==true){
            cout << "\t --- > This should be the only epidemic drawn in this chunk!!!! < -----\n";
        }
    }

    cout << "======= I will draw through the entirety of the THIRD chunk in sequence:\n";
    for(int i=1;i<my_deck._chunk_size()+1;i++){
        Decks::PlayerCard my_card = my_deck.draw();
        cout << "\tCard " << i+2*my_deck._fat_chunk_size() << " is " << my_card.name;
        cout << " \t\t (" << my_deck._total_cards_drawn() << " total player cards drawn during play now.)";
        cout << " \t\t (" << my_deck._remaining_nonepi_cards()+4 -my_deck._epidemics_drawn() << " player cards left to draw)\n";
        if(my_card.epidemic==true){
            cout << "\t --- > This should be the only epidemic drawn in this chunk!!!! < -----\n";
        }
    }

    cout << "======= I will draw through the entirety of the FOURTH chunk in sequence:\n";
    for(int i=1;i<my_deck._chunk_size()+1;i++){
        Decks::PlayerCard my_card = my_deck.draw();
        cout << "\tCard " << i+2*my_deck._fat_chunk_size() + my_deck._chunk_size() << " is " << my_card.name;
        cout << " \t\t (" << my_deck._total_cards_drawn() << " total player cards drawn during play now.)";
        cout << " \t\t (" << my_deck._remaining_nonepi_cards()+4 -my_deck._epidemics_drawn() << " player cards left to draw)\n";
        if(my_card.epidemic==true){
            cout << "\t --- > This should be the only epidemic drawn in this chunk!!!! < -----\n";
        }
    }
}