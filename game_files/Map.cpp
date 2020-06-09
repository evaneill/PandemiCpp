#include <iostream>
#include <string>
#include <vector>

#include "Map.h"

Map::City::City(int ind, int pop, int col, std::string _name,std::vector<int> _neighbors):
	index(ind),
	population(pop),
	color(col),
	name(_name),
	neighbors(_neighbors){}

Map::City::City(){
	index = -1;
	population = -1;
	color = -1;
	name = "";
	neighbors = {};
}