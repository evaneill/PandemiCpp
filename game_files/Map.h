#include <iostream>
#include <set>
#include <string>
#include <vector>

namespace Map
{
	const int BLUE = 0;
	const int YELLOW = 1;
	const int BLACK = 2;
	const int RED = 3;

	class City{

	public:

		int population;
		int index;
		std::string name;
		int color;

		std::set<int> neighbors;

	City(int ind, int pop, int color, std::string name);

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
	
	class Cities{

		std::vector<City> cities;

	public:
		Cities();
		City get_city(int city_index);
	};
}
