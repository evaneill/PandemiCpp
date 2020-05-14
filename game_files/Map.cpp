#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>

#include "Map.h"

Map::City::City(int ind, int pop, int col, std::string _name){
    index = ind;
    population = pop;
    color = col;
    name = _name;
}

Map::Cities::Cities(){

	// shoutout to authors here https://github.com/BlopaSc/PAIndemic for codifying card populations, seemingly not available anywhere else without a physical game.
	// As of 5/14 the connections have been checked city by city against game board and validated.
    cities = {
    
		City(0, 5864000, BLUE, "San Francisco"),
		City(1, 9121000, BLUE, "Chicago" ),
		City(2, 3429000, BLUE, "Montreal" ),
		City(3, 4715000, BLUE, "Atlanta" ),
		City(4, 4679000, BLUE, "Washington"),
		City(5, 20464000, BLUE, "New York" ),
		City(6, 8568000, BLUE, "London" ),
		City(7, 5427000, BLUE, "Madrid" ),
		City(8, 10755000, BLUE, "Paris" ),
		City(9, 5232000, BLUE, "Milan" ),
		City(10, 575000, BLUE, "Essen" ),
		City(11, 4879000, BLUE, "St. Petersburg" ),

		City(12, 14900000, YELLOW, "Los Angeles" ),
		City(13, 19463000, YELLOW, "Mexico City" ),
		City(14, 5582000, YELLOW, "Miami" ),
		City(15, 8702000, YELLOW, "Bogota" ),
		City(16, 9121000, YELLOW, "Lima" ),
		City(17, 6015000, YELLOW, "Santiago" ),
		City(18, 20186000, YELLOW, "Sao Paulo" ),
		City(19, 13639000, YELLOW, "Buenos Aires" ),
		City(20, 11547000, YELLOW, "Lagos" ),
		City(21, 9046000, YELLOW, "Kinshasa" ),
		City(22, 4887000, YELLOW, "Khartoum" ),
		City(23, 3888000, YELLOW, "Johannesburg" ),

		City(24, 2946000, BLACK, "Algiers" ),
		City(25, 13576000, BLACK, "Istanbul" ),
		City(26, 15512000, BLACK, "Moscow" ),
		City(27, 14718000, BLACK, "Cairo" ),
		City(28, 6204000, BLACK, "Baghdad" ),
		City(29, 7419000, BLACK, "Tehran" ),
		City(30, 5037000, BLACK, "Riyadh" ),
		City(31, 20711000, BLACK, "Karachi" ),
		City(32, 22242000, BLACK, "Delhi" ),
		City(33, 16910000, BLACK, "Mumbai" ),
		City(34, 14374000, BLACK, "Kolkata" ),
		City(35, 8865000, BLACK, "Chennai" ),

		City(36, 3785000, RED, "Sydney" ),
		City(37, 26063000, RED, "Jakarta" ),
		City(38, 20767000, RED, "Manila" ),
		City(39, 8314000, RED, "Ho Chi Minh City" ),
		City(40, 7151000, RED, "Bangkok" ),
		City(41, 7106000, RED, "Hong Kong" ),
		City(42, 8338000, RED, "Taipei" ),
		City(43, 13482000, RED, "Shanghai" ),
		City(44, 2871000, RED, "Osaka" ),
		City(45, 17311000, RED, "Beijing" ),
		City(46, 22547000, RED, "Seoul" ),
		City(47, 13189000, RED, "Tokyo" ),

	};

	cities[0].neighbors = {1,12,47,38};
	cities[1].neighbors = {0,2,3,12,13};
	cities[2].neighbors = {1,4,5};
	cities[3].neighbors = {1,4,14};
	cities[4].neighbors = {2,3,5,14};
	cities[5].neighbors = {2,4,6,7};
	cities[6].neighbors = {5,7,8,10};
	cities[7].neighbors = {5,6,8,18,24};
	cities[8].neighbors = {6,7,9,10,24};
	cities[9].neighbors = {8,10,25};
	cities[10].neighbors = {6,8,9,11};
	cities[11].neighbors = {10,25,26};

	cities[12].neighbors = {0,1,13,36};
	cities[13].neighbors = {1,12,14,15,16};
	cities[14].neighbors = {3,4,13,15};
	cities[15].neighbors = {13,14,16,18,19};
	cities[16].neighbors = {13,15,17};
	cities[17].neighbors = {16};
	cities[18].neighbors = {15,19,7,20};
	cities[19].neighbors = {15,18};
	cities[20].neighbors = {18,21,22};
	cities[21].neighbors = {20,22,23};
	cities[22].neighbors = {20,21,23,27};
	cities[23].neighbors = {21,22};

	cities[24].neighbors = {7,8,25,27};
	cities[25].neighbors = {9,11,24,26,27,28};
	cities[26].neighbors = {11,25,29};
	cities[27].neighbors = {24,25,28,30,22};
	cities[28].neighbors = {25,27,29,30,31};
	cities[29].neighbors = {26,28,31,32};
	cities[30].neighbors = {27,28,31};
	cities[31].neighbors = {28,29,30,32,33};
	cities[32].neighbors = {29,31,33,34,35};
	cities[33].neighbors = {31,32,35};
	cities[34].neighbors = {32,35,40,41};
	cities[35].neighbors = {32,33,34,40,37};

	cities[36].neighbors = {37,38,12};
	cities[37].neighbors = {35,40,39,36};
	cities[38].neighbors = {39,41,42,36,0};
	cities[39].neighbors = {37,40,41,38};
	cities[40].neighbors = {34,35,37,39,41};
	cities[41].neighbors = {34,38,39,40,42,43};
	cities[42].neighbors = {38,41,43,44};
	cities[43].neighbors = {45,46,47,41,42};
	cities[44].neighbors = {42,47};
	cities[45].neighbors = {43,46};
	cities[46].neighbors = {45,43,47};
	cities[47].neighbors = {46,43,44,0};

}

Map::City Map::Cities::get_city(int c){
	return cities[c];
}