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
		std::string name="";
		bool epidemic=false;
		bool event=false;
		int index=-1;
		int color=-1; // only instantiated for CityCard
		int population=-1; // only instantiated for CityCard
		PlayerCard(int card_idx);
		PlayerCard()=default;

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
		~PlayerDeck(){};

		// These two methods in combination make a card.
		PlayerCard draw(bool setup=false); // puts two methods below together.
		int draw_index(bool setup);  // an index according to stochastic rules of deck organization.
		PlayerCard draw_inplace(); // produce a card that could be drawn, but don't remove it from the deck

		void update(PlayerCard card); // for forcing the deck to update it's status according to a drawn card (i.e. in collaboration with a card produced by draw_inplace())
		
		PlayerCard make_card_by_vector_index(int index,bool setup=false); // make a card from its index in remaining_nonepi_cards
		PlayerCard make_card_by_indices(int vec_index, int card_index, bool setup=false); // Actually instantiates cards

		void setup_shuffle_deck(); // used after ever player has their cards.
		
		// This tracks losing status - very important! Determines winning/losing status.
		bool isempty();
		int remaining_cards();

		// track whether it's possible for the next card to be an epidemic
		bool epidemic_possible();

		// right now these are purely for testing purposes.
		int _remaining_nonepi_cards();
		int _drawn_cards();
		int _chunk_size();
		int _fat_chunk_size();
		int _total_cards_drawn();
		int _remainder();
		int& _epidemics_drawn();

	};

	class CityCard: public PlayerCard{
	public:
		CityCard(Map::City &city);
		CityCard(Map::City city);
		CityCard(int city_idx);
		~CityCard(){};
		int population;

		// logical == and !=
		const bool operator == (CityCard & rhs) {return index == rhs.index && name == rhs.name && epidemic== rhs.epidemic && event==rhs.event && color==rhs.color;};
		const bool operator != (CityCard & rhs) {return index != rhs.index || name != rhs.name || epidemic!= rhs.epidemic || event!=rhs.event || color!=rhs.color;};
	};

	class EpidemicCard: public PlayerCard{
	// Will have to use board-level logic to resolve.
	public:
		EpidemicCard(int index);
		~EpidemicCard(){};
	};
	
	class EventCard: public PlayerCard{

	public:
		EventCard(int index);
		~EventCard(){};
	};

	// All declarations for Infect Deck 
	class InfectCard{
	public:
		InfectCard(Map::City city);
		InfectCard(int city_idx);
		~InfectCard(){};
		int index;
		int color;
		std::string name;

	};

	class InfectCardGroup{
		public:
			InfectCardGroup(std::vector<InfectCard> _cards);
			~InfectCardGroup(){};
			std::vector<InfectCard> cards;
	};

	class InfectDeck{

		// This vector stores a pop-able stack of distinct chunks of cards
		// Each such group uniquely determines the distribution from which cards are drawn.
		std::vector<InfectCardGroup> deck_stack;
		std::vector<InfectCard> current_discard;

	public:
		InfectDeck();
		~InfectDeck(){}
		InfectCard draw();// return a card that could be drawn, removing it from the deck
		InfectCard draw_inplace(); // return a card that could be drawn, without removing it from the deck
		
		InfectCard draw_bottom(); // for epidemics
		InfectCard draw_bottom_inplace(); 

		void update(InfectCard card,bool bottom=false); // for forcing an infect deck to put this on the discard and pull it from the top group (i.e for use in collaboration with draw_inplace)

		int top_group_size(bool top=true); // number of infect cards in the top infect-card group
		
		void readd_discard();
	};

}

#endif