#include <iostream>
#include <vector>
#include <ctime>
#include <cmath>
#include <algorithm>
#include <random>

#include "Map.h"
#include "Decks.h"

std::string Decks::CARD_NAME(int card_index){
    if(card_index<Map::CITIES.size()){
        return Map::CITY_NAME(card_index);
    } else if(card_index<Map::CITIES.size()+3){
        switch(card_index){
            case 48:
                return "Quiet Night";
                break;
            case 49:
                return "Government Grant";
                break;
            case 50:
                return "Airlift";
                break;
            default:
                return "";
                break;
        }
    } else {
        return "Epidemic";
    }
}

int Decks::CARD_COLOR(int card_index){
    if(card_index<Map::CITIES.size()){
        return Map::CITY_COLOR(card_index);
    } else {
        return -1;
    }
}

bool Decks::IS_EPIDEMIC(int card_index){
    return card_index>=Map::CITIES.size()+3;
}

bool Decks::IS_EVENT(int card_index){
    return card_index>=Map::CITIES.size() && !Decks::IS_EPIDEMIC(card_index);
}

int Decks::POPULATION(int card_index){
    if(card_index<Map::CITIES.size()){
        return Map::CITY_POP(card_index);
    } else {
        return -1;
    }
}

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

    // to be initialized by setup_shuffle_deck()
    deck_chunk_sizes = {};

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
    for(int chunk=0; chunk<difficulty; chunk++){
        if(chunk<(difficulty - remainder) || remainder==0){
            deck_chunk_sizes.push_back(chunk_size);
        } else {
            deck_chunk_sizes.push_back(fat_chunk_size);
        }
    }
}

int Decks::PlayerDeck::draw_index(bool setup){
    // Goal is to get and return the index within remaining_nonepi_cards to be given 
    
    int& chunk_cards_left = deck_chunk_sizes.back();
    if(epidemics_drawn==(difficulty - deck_chunk_sizes.size()) && !setup){
        // number left includes epidemic card
        if((float) rand()< (float) (RAND_MAX/(chunk_cards_left))){
            return 48+3+epidemics_drawn;
        } else {
            return rand() % remaining_nonepi_cards.size();
        }
    } else {
        return rand() % remaining_nonepi_cards.size();
    }
    return -1;
}

int Decks::PlayerDeck::draw(bool setup){
    int drop_idx = draw_index(setup);
    if(drop_idx>=51){
        update(drop_idx,setup);
        return drop_idx;
    } else {
        int card = remaining_nonepi_cards[drop_idx];
        update(card,setup);
        return card;
    }
    // return drawn_card;
}

int Decks::PlayerDeck::draw_inplace(){
    int drop_idx = draw_index(false);

    if(drop_idx>=51){
        return drop_idx;
    } else {
        return remaining_nonepi_cards[drop_idx];
    }
}

void Decks::PlayerDeck::update(int card,bool setup){

    if(card>=51){
        // either increment epidemics drawn
        epidemics_drawn++;
        total_cards_drawn++;
    } else {
        // or remove it from the remaining_nonepi_cards
        total_cards_drawn++;
        for(int ind=0;ind<remaining_nonepi_cards.size();ind++){
            if(remaining_nonepi_cards[ind]==card){
                remaining_nonepi_cards.erase(remaining_nonepi_cards.begin() + ind);
                break;
            }
        }
    }

    if(!setup){
        // if being called during setup, then this isn't initialized yet (or shouldn't be), so ignore it
        deck_chunk_sizes.back()--;
        if(deck_chunk_sizes.back()==0){
            deck_chunk_sizes.pop_back();
        }
    }
    
    // insert it into drawn_cards
    drawn_cards.push_back(card);
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

bool Decks::PlayerDeck::epidemic_possible(){
    // Replicate a little bit of logic from draw_index
    return epidemics_drawn==(difficulty - deck_chunk_sizes.size());
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

Decks::InfectDeck::InfectDeck(){
    // phattest because it's the phattest InfectCardGroup there will be during the game
    std::vector<int> phattest_stack = {};
    for(int c=0;c<Map::CITIES.size();c++){
        phattest_stack.push_back(c);
    }

    // The Deck will be represented by one group of cards, which contains all of them.
    deck_stack.push_back(phattest_stack);
}

void Decks::InfectDeck::readd_discard(){

    std::vector<int> newest_group = current_discard;

    deck_stack.push_back(newest_group);

    current_discard.clear();
}

int Decks::InfectDeck::draw(){
    
    std::vector<int>& current_stack = deck_stack.back();

    // 
    int chosen_index = rand() % current_stack.size();

    int chosen_card = current_stack[chosen_index];
    current_stack.erase(current_stack.begin()+chosen_index);

    if(current_stack.empty()){
        // If we just removed the last card from the most recently added group, get rid of it.
        deck_stack.pop_back();
    }

    current_discard.push_back(chosen_card);

    return chosen_card;
}

int Decks::InfectDeck::draw_inplace(){
    
    std::vector<int>& current_stack = deck_stack.back();

    int chosen_index = rand() % current_stack.size();

    int chosen_card = current_stack[chosen_index];

    // No erasure or discard has been done at this point.
    return chosen_card;
}

void Decks::InfectDeck::update(int card,bool bottom){
    // remove it from the top/bottom group depending
    std::vector<int>& current_stack = bottom ? deck_stack[0] : deck_stack.back();

    for(int ind=0;ind<current_stack.size();ind++){
        if(current_stack[ind]==card){
            current_stack.erase(current_stack.begin()+ind);
            break;
        }
    }
    // always check top stack (it's the only one we'd want to pop) for emptiness
    if(deck_stack.back().empty()){
        deck_stack.pop_back();
    }

    // add it to the discard no matter what
    current_discard.push_back(card);
}

int Decks::InfectDeck::draw_bottom(){
    
    std::vector<int>& current_stack = deck_stack[0];
    
    // 
    int chosen_index = rand() % current_stack.size();

    int chosen_card = current_stack[chosen_index];
    current_stack.erase(current_stack.begin()+chosen_index);

    if(current_stack.empty()){
        // If we just removed the last card from the original bottom of the infect deck, remove it (this might be impossible?)
        deck_stack.erase(deck_stack.begin()+0);
    }

    current_discard.push_back(chosen_card);

    return chosen_card;
}

int Decks::InfectDeck::draw_bottom_inplace(){
    
    std::vector<int>& current_stack = deck_stack[0];
    
    int chosen_index = rand() % current_stack.size();

    int chosen_card = current_stack[chosen_index];

    return chosen_card;
}

int Decks::InfectDeck::top_group_size(bool top){
    if(top){
        return deck_stack.back().size();
    } else {
        return deck_stack[0].size();
    }
    
}