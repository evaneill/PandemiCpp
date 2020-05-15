#include <iostream>
#include <vector>
#include <string>
#include <unordered_set>
#include <ctime>
#include <random>

#include "../game_files/Map.h"

int main(){
	// int current_time = time(NULL);
	// std::cout << "Seed time: " << current_time << "\n";
	std::srand(static_cast<unsigned int>(std::time(nullptr))); 

	int random_int = rand(); // <- This isn't random on first draw. It's exactly equal to seed*16807 every goddamn time, and so cannot be used for my purposes. 
	int successes;
	int trials = 10000;
	for(unsigned int denominator=1;denominator<20;denominator++){
		successes=0;
		for(int trial=0;trial<trials;trial++){
			// unsigned int random_int2 = rand();
			// (rand()/RAND_MAX) < (1/denominator) -> (rand()*denominator) < RAND_MAX
			if((float) rand()< (float) (RAND_MAX/denominator)){
				successes++;
			}
		}
		std::cout<< "With Pr(success) = 1/" << denominator << ", " << successes << " out of " << trials << " trials were successful.\n";
	}
}
