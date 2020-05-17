#include <iostream>
#include <string>
#include <vector>

#include "Map.h"

using namespace Map;

City::City(int ind, int pop, int col, std::string _name,std::vector<int> _neighbors):
	index(ind),
	population(pop),
	color(col),
	name(_name),
	neighbors(_neighbors){}