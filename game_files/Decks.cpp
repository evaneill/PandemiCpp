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
        return Map::CITIES[card_index].name;
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
        return Map::CITIES[card_index].color;
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
        return Map::CITIES[card_index].population;
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

int Decks::PlayerDeck::draw(bool setup){
    int drop_idx = draw_index(setup);
    int drawn_card = make_card_by_vector_index(drop_idx,setup);
    return drawn_card;
}

int Decks::PlayerDeck::draw_inplace(){
    int drop_idx = draw_index(false);

    int idx;
    if(drop_idx>Map::CITIES.size()){
        idx= drop_idx;
    } else {
        idx = remaining_nonepi_cards[drop_idx];
    }

    return idx;
}

void Decks::PlayerDeck::update(int card){
    // remove it from the remaining_nonepi_cards
    
    // Incrementing total_cards_drawn implicitly implies 
    if(card>=51){
        epidemics_drawn++;
        total_cards_drawn++;
    } else {
        total_cards_drawn++;
        for(int ind=0;ind<remaining_nonepi_cards.size();ind++){
            if(remaining_nonepi_cards[ind]==card){
                remaining_nonepi_cards.erase(remaining_nonepi_cards.begin() + ind);
                break;
            }
        }
    }

    // insert it into drawn_cards
    drawn_cards.insert(card);
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
    // Replicate a lot of the logic for drawing indices, but only to determine if epidemic is possible
    int accounted_for_cards=0;

    for(int chunk=0;chunk<difficulty;chunk++){
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
                return false;
            } else {
                return true;
            }
        } else {
            accounted_for_cards+=this_chunk_size;
        }
    }
}

int Decks::PlayerDeck::make_card_by_vector_index(int drop_index,bool setup){

    int idx;
    if(drop_index>Map::CITIES.size() && !setup){
        idx= drop_index;
    } else {
        idx = remaining_nonepi_cards[drop_index];
    }

    return make_card_by_indices(drop_index, idx,setup);
}

int Decks::PlayerDeck::make_card_by_indices(int drop_index, int idx, bool setup){
    
    drawn_cards.insert(idx);
    if(!setup){
        total_cards_drawn++;
    }
    if(idx>=51){
        epidemics_drawn++;
    } else {
        remaining_nonepi_cards.erase(remaining_nonepi_cards.begin()+drop_index);
    }
    return idx;
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