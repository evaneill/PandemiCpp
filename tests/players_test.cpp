#include <iostream>

#include "../game_files/Players.h"

using namespace std;

int main(){
    std::cout << "Building map...\n";
    Map::Cities cities = Map::Cities();
    std::cout << "Successfully built map.\n\n";

    cout <<  "=== Let's make a Quarantine Specialist ===\n";
    Players::Player qs = Players::Player(0,cities);
    cout << "... success! Their attributes are: \n";
    cout << "---- role.name: \t" << qs.role.name << "\n";
    cout << "---- role.required_cure_cards: " << qs.role.required_cure_cards << "\n";
    cout << "---- hand has size: " << qs.hand.size() << "\n";
    cout << "---- position.index: " << qs.position.index << "\n";
    cout << "---- position.name: " << qs.position.name << "\n\n";

    cout <<  "=== Let's make a Medic ===\n";
    Players::Player medic = Players::Player(1,cities);
    cout << "... success! Their attributes are: \n";
    cout << "---- role.name: \t" << medic.role.name << "\n";
    cout << "---- role.required_cure_cards: " << medic.role.required_cure_cards << "\n";
    cout << "---- hand has size: " << medic.hand.size() << "\n";
    cout << "---- position.index: " << medic.position.index << "\n";
    cout << "---- position.name: " << medic.position.name << "\n\n";

    cout <<  "=== Let's make a Scientist ===\n";
    Players::Player scientist = Players::Player(2,cities);
    cout << "... success! Their attributes are: \n";
    cout << "---- role.name: \t" << scientist.role.name << "\n";
    cout << "---- role.required_cure_cards: " << scientist.role.required_cure_cards << "\n";
    cout << "---- hand has size: " << scientist.hand.size() << "\n";
    cout << "---- position.index: " << scientist.position.index << "\n";
    cout << "---- position.name: " << scientist.position.name << "\n\n";

    cout <<  "=== Let's make a Researcher ===\n";
    Players::Player researcher = Players::Player(3,cities);
    cout << "... success! Their attributes are: \n";
    cout << "---- role.name: \t" << researcher.role.name << "\n";
    cout << "---- role.required_cure_cards: " << researcher.role.required_cure_cards << "\n";
    cout << "---- hand has size: " << researcher.hand.size() << "\n";
    cout << "---- position.index: " << researcher.position.index << "\n";
    cout << "---- position.name: " << researcher.position.name << "\n\n";

    cout <<  "=== Let's make a Operations Expert ===\n";
    Players::Player oe = Players::Player(4,cities);
    cout << "... success! Their attributes are: \n";
    cout << "---- role.name: \t" << oe.role.name << "\n";
    cout << "---- role.required_cure_cards: " << oe.role.required_cure_cards << "\n";
    cout << "---- hand has size: " << oe.hand.size() << "\n";
    cout << "---- position.index: " << oe.position.index << "\n";
    cout << "---- position.name: " << oe.position.name << "\n\n";
}