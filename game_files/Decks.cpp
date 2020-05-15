#include <iostream>
#include <vector>
#include <ctime>
#include <cmath>
#include <algorithm>
#include <random>

#include "Map.h"
#include "Decks.h"

Decks::EventCard::EventCard(int ind, std::string _name){
    index = ind;
    name = _name;
    epidemic=false;
}

Decks::EpidemicCard::EpidemicCard(int ind){
    index = ind;
    epidemic=true;
    name = "Epidemic";
}

Decks::CityCard::CityCard(Map::City &city){
    color = city.color;
    name = city.name;
    index = city.index;
    epidemic = false;
}

// Setting up the game deck steps
//  1. Initialize deck w/ the constructor
//  2. for each player, 
//      use draw_setup_cards(# cards per player) to retrieve player hands during setup.
//  3. setup_shuffle_deck() to set parameters that can track probability distribution over each draw().

Decks::PlayerDeck::PlayerDeck(int diff, Map::Cities set_map): fixed_board(set_map){
    // Takes difficulty in 4,5,6 but you could throw whatever garbage in here you want
    // input an alread-instantiated map set_map. Referenced here.
    // 
    // In the real game, epidemics are shuffled into equal-ish sized chunks of cards and the chunks are stacked together
    // Here the deck is modeled as drawing from a uniform distribution over the possible remaining cards in each chunk.
    //
    // For example:
    //     - Let's say each "chunk" is of size 13 (including the one epidemic card). 
    //     - We've drawn 4 cards and haven't seen an epidemic drawn yet. 
    //     - Then the next draw has a (1/9) chance of being an epidemic,
    //     and (8/9)*(1/47) chance of being any remaining player card.
    //     - This is because there are 9 cards left in the chunk, 1 of which must be an epidemic
    //     - Of the 47 = 51-4 undrawn player cards, all of them are equally likely!
    //
    // This is complicated by the fact that the deck doesn't divide evenly.
    // Every chunk is of size floor[(51+difficulty)/difficulty] or floor[(51+difficulty)/difficulty]+1 ("fat"). 
    // I put the chunks of larger size FIRST when defining the probabilities. This is a little bit of a thumb on the scale, but barely.
    drawn_cards = {};
    remaining_nonepi_cards = {};
    for (int i=0;i<51;i++){
        remaining_nonepi_cards.push_back(i);
    }
    // total cards drawn during play. Only starts to increment after setup. 
    total_cards_drawn = 0;

    epidemics_drawn = 0;
    difficulty = diff;
    // index 0-47 city cards corresponding precisely to Map::City indices
    // index 48-50 event cards
    // index 51+ epidemic cards
}

std::vector<Decks::PlayerCard> Decks::PlayerDeck::draw_setup_cards(int num_cards){
    // To be called for each player, only during game setup.
    // Returns vector of ints representing card.index that can be turned into player hands.
    std::vector<PlayerCard> return_vector (num_cards);

    // For each card to be drawn for this player...
    std::cout << "Drawing card \n";
    for(int k=0;k<num_cards;k++){
        std::cout << k << " ";
        // Choose the card to pull out of the non-epidemic cards
        int chosen_index = rand() % remaining_nonepi_cards.size();

        // Put it into the output to be returned.
        // The make_card...() does the insertion into drawn cards and removal from remaining.
        std::cout << ", make a new card, ";
        Decks::PlayerCard new_card = make_card_by_card_index(chosen_index);
        std::cout << "made a new card, will put it into return_vector, ";

        return_vector[k]=new_card;
        std::cout << "did put it onto return_vector.\n";
    }
    std::cout << "\n";
    total_cards_drawn = 0; // This has to be set back to 0 for remaining gameplay logic to work.
    return return_vector;
}

void Decks::PlayerDeck::setup_shuffle_deck(){
    // This should be the only time these variables are changed. Will be called after each player 
    // The "shuffle" part of the name is an illusion whose function is simulated by the logic of draw()
    remainder = (remaining_nonepi_cards.size()+difficulty) % difficulty;
    if(remainder == 0){
        // If the cards are divisible into equal-sized chunks...
        fat_chunk_size = (remaining_nonepi_cards.size()+difficulty)/difficulty;
        chunk_size = fat_chunk_size;
    } else {
        // some chunks will  be 1 card larger than others
        fat_chunk_size = std::floor((remaining_nonepi_cards.size()+difficulty)/difficulty)+1;
        chunk_size = fat_chunk_size-1; 

    }
}

int Decks::PlayerDeck::draw_index(){
    // Goal is to get and return the card index to be given 
    
    int accounted_for_cards=0; // cards that have preceded the considered chunk in the iteration.
    int idx=-1;             // Initialize randomly collected element to -1. Points to card index used to initialize a card.
    int drop_index = -1;    // The index that's randomly generated to pull out a vector element or epidemic card.
    for(int chunk=0;chunk<difficulty;chunk++){
        // find out whether this chunk is a fatty or not
        int this_chunk_size;
        if(chunk+1>remainder){
            this_chunk_size = chunk_size;
        } else{
            this_chunk_size = fat_chunk_size;
        }

        // Use accumulator accounted_for_cards to walk through chunks until you see which one you're in.
        // I don't like this though... seems like there should be some way to keep a few class variables that track something like this.
        if(total_cards_drawn>=accounted_for_cards && total_cards_drawn<(this_chunk_size + accounted_for_cards)){
            if(epidemics_drawn>chunk){
                // If an epidemic has already been drawn in this chunk...
                //      sample indices from all non-drawn cards uniformly.
                //      prob of any remaining non_epi card = 1/(remaining_nonepi_cards.size());
                drop_index = rand() % remaining_nonepi_cards.size();
                break;
            } else {
                // There is still an epidemic card in this chunk. Probability of drawing ranges in [1/(this_chunk_size),1]
                //      It's probability is (1/(# remaining cards in this chunk))              
                if((float) rand()< (float) (RAND_MAX/(this_chunk_size + accounted_for_cards - total_cards_drawn))){
                    // if a random number in [0,RAND_MAX] is between [0,(RAND_MAX/(# remaining cards in chunk))], draw the next epidemic card.
                    drop_index = 48+3+epidemics_drawn; // draw the indices sequentially, doesn't matter.
                } else{
                    // Otherwise we're drawing uniformly from all non-epidemic cards.
                    drop_index = rand() % remaining_nonepi_cards.size();
                }
                break;
            }
        } else {
            accounted_for_cards+=this_chunk_size;
        }
    }

    return drop_index;
}

// Draw to only be used during play, not setup.
Decks::PlayerCard Decks::PlayerDeck::draw(){
    int drop_idx = draw_index();
    PlayerCard drawn_card = make_card_by_vector_index(drop_idx);
    return drawn_card;
}

Decks::PlayerCard Decks::PlayerDeck::make_card_by_card_index(int idx){
    std::vector<int>::iterator it = std::find(remaining_nonepi_cards.begin(), remaining_nonepi_cards.end(), idx);
    int drop_index = std::distance(remaining_nonepi_cards.begin(), it);
    return make_card_by_indices(drop_index, idx);
}

Decks::PlayerCard Decks::PlayerDeck::make_card_by_vector_index(int drop_index){

    int idx;
    if(drop_index>remaining_nonepi_cards.size()){
        idx= drop_index;
    } else {
        idx = remaining_nonepi_cards[drop_index];
    }

    return make_card_by_indices(drop_index, idx);
}

Decks::PlayerCard Decks::PlayerDeck::make_card_by_indices(int drop_index, int idx){
    
    drawn_cards.insert(idx);
    total_cards_drawn++;

    if(idx>=0 && idx<=47){
        remaining_nonepi_cards.erase(remaining_nonepi_cards.begin()+drop_index);

        Map::City corresponding_city = fixed_board.get_city(idx);

        return (PlayerCard) CityCard(corresponding_city);
    } else if(idx>=48 && idx<=50){
        switch(idx){
            case 48:{
                remaining_nonepi_cards.erase(remaining_nonepi_cards.begin()+drop_index);

                return (PlayerCard) EventCard(idx,"Quiet Night");
                }
            case 49:{
                remaining_nonepi_cards.erase(remaining_nonepi_cards.begin()+drop_index);

                return (PlayerCard) EventCard(idx,"Government Grant");
                }
            case 50:{
                remaining_nonepi_cards.erase(remaining_nonepi_cards.begin()+drop_index);

                return (PlayerCard) EventCard(idx,"Airlift");
                }
            default:
                return (PlayerCard) PlayerCard(); // Should make game fail ASAP hopefully
        }
    } else if(idx>=51){
        epidemics_drawn++;
        return (PlayerCard) EpidemicCard(idx);
    }
    return (PlayerCard) PlayerCard(); // if somehow no logic is caught, try to make game fail.
}

int Decks::PlayerDeck::_remaining_nonepi_cards(){return remaining_nonepi_cards.size();}
int Decks::PlayerDeck::_chunk_size(){return chunk_size;};
int Decks::PlayerDeck::_fat_chunk_size(){return fat_chunk_size;};
int Decks::PlayerDeck::_total_cards_drawn(){return total_cards_drawn;};
int Decks::PlayerDeck::_remainder(){return remainder;}
int Decks::PlayerDeck::_drawn_cards(){return drawn_cards.size();}
int Decks::PlayerDeck::_epidemics_drawn(){return epidemics_drawn;}