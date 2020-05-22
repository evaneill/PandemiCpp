#ifndef DECK_H
#define DECK_H

#include <iostream>
#include <set>
#include <string>
#include <vector>
#include <random>

#include "Map.h"

namespace Decks
{
	// All declarations necessary for player deck

	// Base class
	class PlayerCard{
	public:
		std::string name;
		bool epidemic;
		bool event;
		int index;
		int color; // only instantiated for CityCard
		int population; // only instantiated for CityCard
		PlayerCard() = default;

		const bool operator == (PlayerCard& rhs) {return index == rhs.index && name == rhs.name && epidemic== rhs.epidemic && event==rhs.event;};
	};

	class PlayerDeck{
		std::set<int> drawn_cards;
		std::vector<int> remaining_nonepi_cards;

		// number of epidemic cards (and number of epidemic-holding segments into which deck is segregated)
		int difficulty;

		// These are used to track the exact distribution according to which cards are draw()n.
		int epidemics_drawn; 	// is what it says
		int fat_chunk_size;  	// when # player deck cards after handing out player cards isn't divisible by difficulty, chunks into which epidemics are shuffled are unevenly sized.
		int chunk_size;			// Lesser size of shuffled deck chunk containing an epidemic.
		int remainder;			// Number of "fat chunks": remainder of (# remaining cards after giving player hands sans epidemics)/(difficulty)
		int total_cards_drawn;	// Track cards drawn during play. DIFFERENT from total # of cards missing from deck since player hands have to be created at first.

	public:
		PlayerDeck(int diff); //called on instantiation to start a game. Thereafter only draw() is used, whether in a game copy or  
		PlayerDeck(){};

		// These two methods in combination make a card.
		PlayerCard draw(bool setup=false); // puts two methods below together.
		int draw_index(bool setup);  // an index according to stochastic rules of deck organization.
		
		PlayerCard make_card_by_vector_index(int index,bool setup=false); // make a card from its index in remaining_nonepi_cards
		PlayerCard make_card_by_indices(int vec_index, int card_index, bool setup=false); // Actually instantiates cards

		void setup_shuffle_deck(); // used after ever player has their cards.
		
		// This tracks losing status - very important! Determines winning/losing status.
		bool isempty();

		// right now these are purely for testing purposes.
		int _remaining_nonepi_cards();
		int _drawn_cards();
		int _chunk_size();
		int _fat_chunk_size();
		int _total_cards_drawn();
		int _remainder();
		int _epidemics_drawn();

	};

	class CityCard: public PlayerCard{
	public:
		CityCard(Map::City &city);
		CityCard(Map::City city);
		CityCard(int city_idx);
		int color;
		int population;

		// logical == and !=
		const bool operator == (CityCard & rhs) {return index == rhs.index && name == rhs.name && epidemic== rhs.epidemic && event==rhs.event && color==rhs.color;};
		const bool operator != (CityCard & rhs) {return index != rhs.index || name != rhs.name || epidemic!= rhs.epidemic || event!=rhs.event || color!=rhs.color;};
	};

	class EpidemicCard: public PlayerCard{
	// Will have to use board-level logic to resolve.
	public:
		EpidemicCard(int index);
	};
	
	class EventCard: public PlayerCard{

	public:
		EventCard(int index,std::string name);
	};

	// All declarations for Infect Deck 
	class InfectCard{
	public:
		InfectCard(Map::City city);
		InfectCard(int city_idx);
		int index;
		int color;
		std::string name;

	};

	class InfectCardGroup{
		public:
			InfectCardGroup(std::vector<InfectCard> _cards);
			std::vector<InfectCard> cards;
			InfectCard draw();
	};

	class InfectDeck{

		// This vector stores a pop-able stack of distinct chunks of cards
		// Each such group uniquely determines the distribution from which cards are drawn.
		std::vector<InfectCardGroup> deck_stack;
		std::vector<InfectCard> current_discard;

	public:
		InfectDeck();
		InfectCard draw();
		InfectCard draw_bottom(); // for epidemics
		
		void readd_discard();
	};

}

#endif