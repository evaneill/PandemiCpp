#ifndef MAP_H
#define MAP_H

#include <string>
#include <vector>

namespace Map
{
	// fixed indices of color descriptions
	inline const int BLUE = 0;
	inline const int YELLOW = 1;
	inline const int BLACK = 2;
	inline const int RED = 3;

	// fixed vector of color names
	inline const std::vector<std::string> COLORS = {"BLUE","YELLOW","BLACK","RED"};

	class City{

	public:

		const int population;
		const int index;
		const std::string name;
		const int color;

		const std::vector<int> neighbors;

	City();
	City(int ind, int pop, int color, std::string name,std::vector<int> _neighbors);

	const bool operator == (City &rhs) {return index==rhs.index && population==rhs.population && name==rhs.name && color==rhs.color;}
	const bool operator < (City &rhs) {return index<rhs.index;}
	};

	struct CityHasher
	{		
		size_t operator()(const Map::City &city) const {return std::hash<int>()(city.index);}
		
	};

	struct CityComparator
	{
		bool operator()(const Map::City &city1, const Map::City &city2) const {return city1.index==city2.index && city1.name==city2.name && city1.color==city2.color && city1.population==city2.population;}
	};
	
	inline std::vector<City> CITIES = {
		City(0, 5864000, BLUE, "San Francisco", {1,12,47,38}),
		City(1, 9121000, BLUE, "Chicago" , {0,2,3,12,13}),
		City(2, 3429000, BLUE, "Montreal" , {1,4,5}),
		City(3, 4715000, BLUE, "Atlanta" , {1,4,14}),
		City(4, 4679000, BLUE, "Washington", {2,3,5,14}),
		City(5, 20464000, BLUE, "New York" , {2,4,6,7}),
		City(6, 8568000, BLUE, "London" , {5,7,8,10}),
		City(7, 5427000, BLUE, "Madrid" , {5,6,8,18,24}),
		City(8, 10755000, BLUE, "Paris" , {6,7,9,10,24}),
		City(9, 5232000, BLUE, "Milan" , {8,10,25}),
		City(10, 575000, BLUE, "Essen" , {6,8,9,11}),
		City(11, 4879000, BLUE, "St. Petersburg" ,{10,25,26}),

		City(12, 14900000, YELLOW, "Los Angeles" ,{0,1,13,36}),
		City(13, 19463000, YELLOW, "Mexico City" ,{1,12,14,15,16}),
		City(14, 5582000, YELLOW, "Miami" ,{3,4,13,15}),
		City(15, 8702000, YELLOW, "Bogota" ,{13,14,16,18,19}),
		City(16, 9121000, YELLOW, "Lima" ,{13,15,17}),
		City(17, 6015000, YELLOW, "Santiago" ,{16}),
		City(18, 20186000, YELLOW, "Sao Paulo" ,{15,19,7,20}),
		City(19, 13639000, YELLOW, "Buenos Aires" ,{15,18}),
		City(20, 11547000, YELLOW, "Lagos" ,{18,21,22}),
		City(21, 9046000, YELLOW, "Kinshasa" ,{20,22,23}),
		City(22, 4887000, YELLOW, "Khartoum" ,{20,21,23,27}),
		City(23, 3888000, YELLOW, "Johannesburg" ,{21,22}),

		City(24, 2946000, BLACK, "Algiers" ,{7,8,25,27}),
		City(25, 13576000, BLACK, "Istanbul" ,{9,11,24,26,27,28}),
		City(26, 15512000, BLACK, "Moscow" ,{11,25,29}),
		City(27, 14718000, BLACK, "Cairo" ,{24,25,28,30,22}),
		City(28, 6204000, BLACK, "Baghdad" ,{25,27,29,30,31}),
		City(29, 7419000, BLACK, "Tehran" ,{26,28,31,32}),
		City(30, 5037000, BLACK, "Riyadh" ,{27,28,31}),
		City(31, 20711000, BLACK, "Karachi" ,{28,29,30,32,33}),
		City(32, 22242000, BLACK, "Delhi" ,{29,31,33,34,35}),
		City(33, 16910000, BLACK, "Mumbai" ,{31,32,35}),
		City(34, 14374000, BLACK, "Kolkata" ,{32,35,40,41}),
		City(35, 8865000, BLACK, "Chennai" ,{32,33,34,40,37}),

		City(36, 3785000, RED, "Sydney" , {37,38,12}),
		City(37, 26063000, RED, "Jakarta" , {35,40,39,36}),
		City(38, 20767000, RED, "Manila" , {39,41,42,36,0}),
		City(39, 8314000, RED, "Ho Chi Minh City" , {37,40,41,38}),
		City(40, 7151000, RED, "Bangkok" , {34,35,37,39,41}),
		City(41, 7106000, RED, "Hong Kong" , {34,38,39,40,42,43}),
		City(42, 8338000, RED, "Taipei" , {38,41,43,44}),
		City(43, 13482000, RED, "Shanghai" , {45,46,47,41,42}),
		City(44, 2871000, RED, "Osaka" , {42,47}),
		City(45, 17311000, RED, "Beijing" , {43,46}),
		City(46, 22547000, RED, "Seoul" , {45,43,47}),
		City(47, 13189000, RED, "Tokyo" , {46,43,44,0})
	};
}

#endif