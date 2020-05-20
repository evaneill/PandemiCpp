#include <iostream>
#include <random>
#include <ctime> 

#include "../game_files/Decks.h"

using namespace std;

int main(){
    std::cout << "Retrieving map...\n";
    std::vector<Map::City>& cities = Map::CITIES;
    std::cout << "...success! This map has " << cities.size() << " cities. (Should be 48)\n\n";

    cout << "Trying to initialize an InfectDeck...\n";
    Decks::InfectDeck my_deck = Decks::InfectDeck();
    cout << "...success!\n\n";

    std::srand(static_cast<unsigned int>(std::time(nullptr))); 

	int random_int = rand();
    cout << "First 5 cards: \n";
    for(int k=0;k<5;k++){
        cout << "Card " << k+1;
        Decks::InfectCard drawn_card = my_deck.draw();
        cout << " = " << drawn_card.name << "\n";
    }

    cout << "Now let's put it back on top!\n";
    my_deck.readd_discard();
    cout << "... success!\n\n";

    cout << "The next 5 cards should be ALL THE SAME as those you just saw: \n";
    for(int k=0;k<5;k++){
        cout << "Card " << k+1;
        Decks::InfectCard drawn_card = my_deck.draw();
        cout << " = " << drawn_card.name << "\n";
    }

    cout << "\nThe next 3 cards should not be any of the ones you've just seen:\n";
    for(int k=0;k<3;k++){
        cout << "Card " << k+1;
        Decks::InfectCard drawn_card = my_deck.draw();
        cout << " = " << drawn_card.name << "\n";
    }

    cout << "Making a new deck...\n";
    Decks::InfectDeck new_deck = Decks::InfectDeck();
    cout << "...success!\n\n";

    cout << "First 10 cards: \n";
    for(int k=0;k<10;k++){
        cout << "Card " << k+1;
        Decks::InfectCard drawn_card = new_deck.draw();
        cout << " = " << drawn_card.name << "\n";
    }

    cout << "Now let's put it back on top!\n";
    new_deck.readd_discard();
    cout << "... success!\n\n";

    cout << "Next cards should be 5 of those you just saw: \n";
    for(int k=0;k<5;k++){
        cout << "Card " << k+1;
        Decks::InfectCard drawn_card = new_deck.draw();
        cout << " = " << drawn_card.name << "\n";
    }

    cout << "Now let's put those 5 back on top!\n";
    new_deck.readd_discard();
    cout << "... success!\n\n";

    cout << "These 5 cards should be exactly those you just saw: \n";
    for(int k=0;k<5;k++){
        cout << "Card " << k+1;
        Decks::InfectCard drawn_card =new_deck.draw();
        cout << " = " << drawn_card.name << "\n";
    }

    cout << "These 10 cards should start with 5 of original 10 that weren't originally discarded, then 5 brand new: \n";
    for(int k=0;k<10;k++){
        cout << "Card " << k+1;
        Decks::InfectCard drawn_card =new_deck.draw();
        cout << " = " << drawn_card.name << "\n";
    }



}