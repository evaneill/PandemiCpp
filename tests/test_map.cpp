#include <iostream>
#include <unordered_set>

#include "../game_files/Map.h"

#include <algorithm>
#include <iterator>

void print(std::set<int> const &s)
{
    std::copy(s.begin(),
            s.end(),
            std::ostream_iterator<int>(std::cout, " "));
}

int main(){
    std::cout << "First lets make an example out of oxford: \n";
    Map::City oxford = Map::City(0,100000,Map::BLACK,"Oxford");
    std::cout << "Oxford is defined. Giving oxford neighbors...\n";
    std::set<int> _neighbors = {1,2,3};
    oxford.neighbors = _neighbors;
    std::cout << "Now oxford has neighbors.\n";
    
    std::cout << "Map::City oxford:\n";
    std::cout << "-- index = "<< oxford.index << "\t (should be 0)\n";
    std::cout << "-- population = " << oxford.population << "\t (should be 100000)\n";
    std::cout << "-- color = " << oxford.color << "\t\t (should be " << Map::BLACK << ")\n";
    std::cout << "-- name = " << oxford.name << "\t (should be Oxford)\n";
    std::cout << "-- neighbors = {";
    print(oxford.neighbors);
    std::cout << "}\t (Should be {1 2 3})\n\n";



    std::cout << "Building map...\n";
    Map::Cities cities = Map::Cities();
    std::cout << "Successfully built map.\n\n";

    std::cout << "Let's list out all the city connections: \n";
    for(int i=0;i<48;i++){
        Map::City current_city = cities.get_city(i);
        std::set<int>::iterator it = current_city.neighbors.begin();

        std::cout << current_city.name << " is connected to:\n";
        while(it!=current_city.neighbors.end()){
            std::cout << "-- " << cities.get_city(*it).name << "\n";
            it++;
        }
    }
}