#include <iostream>
#include <vector>
#include <ctime>
#include <cmath>
#include <algorithm>
#include <random>

#include "Map.h"
#include "Decks.h"

Decks::EventCard::EventCard(int ind):PlayerCard(ind){}

Decks::EpidemicCard::EpidemicCard(int ind):PlayerCard(ind){}

Decks::CityCard::CityCard(Map::City &city):PlayerCard(city.index){}

Decks::CityCard::CityCard(Map::City city):PlayerCard(city.index){}

Decks::CityCard::CityCard(int city_idx):PlayerCard(city_idx){}

Decks::PlayerCard::PlayerCard(int card_idx){
    if(card_idx<48){
        name = Map::CITIES[card_idx].name;
        index = Map::CITIES[card_idx].index;
        color = Map::CITIES[card_idx].color;
        population = Map::CITIES[card_idx].population;

        epidemic = false;
        event = false;
    } else if(card_idx>=48 && card_idx<=50){
        index = card_idx;
        color = -1;
        population = -1;

        epidemic = false;
        event = true;
        switch(card_idx){
            case 48:
                name = "Quiet Night";
                break;
            case 49:
                name = "Government Grant";
                break;
            case 50:
                name = "Airlift";
                break;
        }
    } else if(card_idx>=51){
        name = "Epidemic";
        index = card_idx;
        epidemic = true;

        event = false;
        color=-1;
        population=-1;
    }
}
// Player Deck Section
// Setting up the Player deck steps
//  1. Initialize deck w/ the constructor
//  2. for each player, use draw(true) to retrieve player hands during setup. 
//  3. setup_shuffle_deck() to set parameters that can track probability distribution over each draw().


Decks::PlayerDeck::PlayerDeck(int diff){
    // Takes difficulty in 4,5,6 but you could throw whatever garbage in here you want
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

int Decks::PlayerDeck::draw_index(bool setup){
    // Goal is to get and return the index within remaining_nonepi_cards to be given 
    
    int accounted_for_cards=0; // cards that have preceded the considered chunk in the iteration.
    int drop_index;    // The index that's randomly generated to pull out a vector element or epidemic card.
    if(setup){
        return rand() % remaining_nonepi_cards.size();
    }
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
                return rand() % remaining_nonepi_cards.size();
            } else {
                // There is still an epidemic card in this chunk. Probability of drawing ranges in [1/(this_chunk_size),1]
                //      It's probability is (1/(# remaining cards in this chunk))              
                if((float) rand()< (float) (RAND_MAX/(this_chunk_size + accounted_for_cards - total_cards_drawn))){
                    // if a random number in [0,RAND_MAX] is between [0,(RAND_MAX/(# remaining cards in chunk))], draw the next epidemic card.
                    return 48+3+epidemics_drawn; // draw the indices sequentially, doesn't matter.
                } else{
                    // Otherwise we're drawing uniformly from all non-epidemic cards.
                    return rand() % remaining_nonepi_cards.size();
                }
            }
        } else {
            accounted_for_cards+=this_chunk_size;
        }
    }
    return -1;
}

// Draw to only be used during play, not setup.
Decks::PlayerCard Decks::PlayerDeck::draw(bool setup){
    int drop_idx = draw_index(setup);
    Decks::PlayerCard drawn_card = make_card_by_vector_index(drop_idx,setup);
    return drawn_card;
}

bool Decks::PlayerDeck::isempty(){
    if(remaining_cards()>0){
        return false;
    } else {
        return true;
    }
}

int Decks::PlayerDeck::remaining_cards(){
    return remaining_nonepi_cards.size()+difficulty-epidemics_drawn;
}

Decks::PlayerCard Decks::PlayerDeck::make_card_by_vector_index(int drop_index,bool setup){

    int idx;
    if(drop_index>Map::CITIES.size() && !setup){
        idx= drop_index;
    } else {
        idx = remaining_nonepi_cards[drop_index];
    }

    return make_card_by_indices(drop_index, idx,setup);
}

Decks::PlayerCard Decks::PlayerDeck::make_card_by_indices(int drop_index, int idx, bool setup){
    
    drawn_cards.insert(idx);
    if(!setup){
        total_cards_drawn++;
    }
    if(idx>=51){
        epidemics_drawn++;
    } else {
        remaining_nonepi_cards.erase(remaining_nonepi_cards.begin()+drop_index);
    }
    return PlayerCard(idx);
}

// These are only called for testing so far. 
int Decks::PlayerDeck::_remaining_nonepi_cards(){return remaining_nonepi_cards.size();}
int Decks::PlayerDeck::_chunk_size(){return chunk_size;};
int Decks::PlayerDeck::_fat_chunk_size(){return fat_chunk_size;};
int Decks::PlayerDeck::_total_cards_drawn(){return total_cards_drawn;};
int Decks::PlayerDeck::_remainder(){return remainder;}
int Decks::PlayerDeck::_drawn_cards(){return drawn_cards.size();}
int& Decks::PlayerDeck::_epidemics_drawn(){return epidemics_drawn;}

// INFECTION DECK section

Decks::InfectCard::InfectCard(Map::City city){
    index = city.index;
    color = city.color;
    name = city.name;
}

Decks::InfectCard::InfectCard(int city_idx){
    index = Map::CITIES[city_idx].index;
    color = Map::CITIES[city_idx].color;
    name = Map::CITIES[city_idx].name;
}

Decks::InfectDeck::InfectDeck(){
    // phattest because it's the phattest InfectCardGroup there will be during the game
    Decks::InfectCardGroup phattest_stack = InfectCardGroup({});
    for(int c=0;c<Map::CITIES.size();c++){
        phattest_stack.cards.push_back(Decks::InfectCard(Map::CITIES[c]));
    }

    // The Deck will be represented by one group of cards, which contains all of them.
    deck_stack.push_back(phattest_stack);
}

void Decks::InfectDeck::readd_discard(){

    Decks::InfectCardGroup newest_group = Decks::InfectCardGroup(current_discard);

    deck_stack.push_back(newest_group);

    current_discard = {};
}

Decks::InfectCardGroup::InfectCardGroup(std::vector<InfectCard> _cards): cards(_cards){};

Decks::InfectCard Decks::InfectDeck::draw(){
    
    Decks::InfectCardGroup& current_stack = deck_stack.back();

    // 
    int chosen_index = rand() % current_stack.cards.size();

    Decks::InfectCard chosen_card = current_stack.cards[chosen_index];
    current_stack.cards.erase(current_stack.cards.begin()+chosen_index);

    if(current_stack.cards.empty()){
        // If we just removed the last card from the most recently added group, get rid of it.
        deck_stack.pop_back();
    }

    current_discard.push_back(chosen_card);

    return chosen_card;
}

Decks::InfectCard Decks::InfectDeck::draw_bottom(){
    
    Decks::InfectCardGroup& current_stack = deck_stack[0];
    
    // 
    int chosen_index = rand() % current_stack.cards.size();

    Decks::InfectCard chosen_card = current_stack.cards[chosen_index];
    current_stack.cards.erase(current_stack.cards.begin()+chosen_index);

    if(current_stack.cards.empty()){
        // If we just removed the last card from the original bottom of the infect deck, remove it (this might be impossible?)
        deck_stack.erase(deck_stack.begin()+0);
    }

    current_discard.push_back(chosen_card);

    return chosen_card;
}