#include "Hashes.h"

#include <boost/container_hash/hash.hpp>

#include "../../game_files/Board.h"
#include "../../game_files/Decks.h"
#include "../../game_files/Players.h"

using namespace std;

/*
Logical hashes for objects that define the game state, including:
    - The amount and position of disease on the board
    - Number of outbreaks
    - Number of epidemics drawn
    - Number of cured & eradicated diseases
    - Position of research stations
    - Composition of player hands
    - Infection deck discard composition
    - Player deck undrawn cards
*/

size_t Hashes::boardHash(Board::Board& board){
    size_t seed = 0;
    boost::hash_combine(seed,(unsigned long long) Hashes::VecHash(board.get_stations()));
    boost::hash_combine(seed,(unsigned long long) Hashes::playerHash(board.get_players())); 
    boost::hash_combine(seed,(unsigned long long) Hashes::VecHash(board.get_playerdeck_remaining_cards()));
    // boost::hash_combine(seed,Hashes::VecVecHash(board.get_infectdeck_stack())); // does this even need to be hashed? Out of player control. Mad a decision not to.
    boost::hash_combine(seed,board.get_outbreak_count());
    boost::hash_combine(seed,board.get_epidemic_count());
    boost::hash_combine(seed,(unsigned long long) Hashes::ArrayArrayHash(board.get_disease_count()));
    // boost::hash_combine(seed,(unsigned long long) Hashes::VecHash(board.get_cure_status()));
    // boost::hash_combine(seed,(unsigned long long) Hashes::VecHash(board.get_eradicated()));

    return seed;
}

size_t Hashes::playerHash(const std::vector<Players::Player>& players){
    size_t out = 0;
    for(Players::Player p: players){
        boost::hash_combine(out,(unsigned long long) Hashes::VecHash(p.get_all_cards())); // Hash the _ordered_ hand of the player. To ensure that just because order has changed, hash stays the same
    }
    return out;
}

size_t Hashes::VecHash(const std::vector<int>& vec){
    size_t seed = 0;
    for(const auto& v : vec){
        boost::hash_combine(seed,v);
    }
    return seed;
}

size_t Hashes::VecHash(const std::vector<bool>& vec){
    size_t seed = 0;
    for(const auto& v : vec){
        if(v){
            boost::hash_combine(seed,1);
        } else {
            boost::hash_combine(seed,0);
        }
    }
    return seed;
}

size_t Hashes::ArrayArrayHash(const std::array<std::array<int,48>,4>& multiarray){
    size_t seed = 0;
    for(const auto& a: multiarray){
        for(const auto& b: a){
            boost::hash_combine(seed,b);// b should be an int
        }
    }
    return seed;
}