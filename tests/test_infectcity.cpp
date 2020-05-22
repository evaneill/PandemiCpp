#include <array>
#include <iostream>

#include "../game_files/Board.h"
#include "../game_files/Decks.h"

using namespace std;

int main(){
    cout << "Making a board...";
    Board::Board new_board = Board::Board({0,2,3},4);
    cout << "success!" << endl;

    cout << "Reset disease count...";
    new_board.reset_disease_count();
    cout << "success!" << endl;

    cout << "Disease count safe: " << new_board.disease_count_safe() << endl;
    std::array<std::array<int,48>,4>& disease_count = new_board.get_disease_count(); // reference to disease count

    cout << "CITY DISEASE COUNTS: " << endl;
    for(int city=0;city<Map::CITIES.size();city++){
        cout << Map::CITIES[city].name << ": "<< endl;
        for(int col=0;col<4;col++){
            cout << Map::COLORS[col] << "=" << disease_count[col][city] << "\t";
        }
        cout << endl;
    }   

    cout << "Infect San Francisco with 1 blue cube...";
    new_board.infect(0,0,1);
    cout << "...success!"<< endl;

    cout << Map::CITIES[0].name << " has " << disease_count[Map::BLUE][0] << " cube" << endl;

    cout << "Add another 2 blue cubes...";
    new_board.infect(0,0,2);
    cout << "...success!"<< endl;

    cout << Map::CITIES[0].name << " has " << disease_count[Map::BLUE][0] << " cubes" << endl;

    cout << endl << "Try adding one more to get an outbreak!";
    new_board.infect(0,0,1);
    cout << "...success!"<< endl;

    cout << "Outbreak count = " << new_board.get_outbreak_count() << endl;

    cout << endl << Map::CITIES[0].name << " has " << disease_count[Map::BLUE][0] << " cubes" << endl;
    for(int neighbor: Map::CITIES[0].neighbors){
        cout << Map::CITIES[neighbor].name << " has " << disease_count[Map::BLUE][neighbor] << " BLUE cubes" << endl;
        cout << Map::CITIES[neighbor].name << " has " << disease_count[Map::YELLOW][neighbor] << " YELLOW cubes" << endl;
        cout << Map::CITIES[neighbor].name << " has " << disease_count[Map::BLACK][neighbor] << " BLACK cubes" << endl;
        cout << Map::CITIES[neighbor].name << " has " << disease_count[Map::RED][neighbor] << " RED cubes" << endl;
    }

    cout << endl << "Outbreak count = " << new_board.get_outbreak_count() << endl;

    cout << endl << "Try adding one more to get another outbreak!";
    new_board.infect(0,0,1);
    cout << "...success!"<< endl;
    cout << endl << Map::CITIES[0].name << " has " << disease_count[Map::BLUE][0] << " cubes" << endl;
    for(int neighbor: Map::CITIES[0].neighbors){
        cout << Map::CITIES[neighbor].name << " has " << disease_count[Map::BLUE][neighbor] << " BLUE cubes" << endl;
        cout << Map::CITIES[neighbor].name << " has " << disease_count[Map::YELLOW][neighbor] << " YELLOW cubes" << endl;
        cout << Map::CITIES[neighbor].name << " has " << disease_count[Map::BLACK][neighbor] << " BLACK cubes" << endl;
        cout << Map::CITIES[neighbor].name << " has " << disease_count[Map::RED][neighbor] << " RED cubes" << endl;
    }

    cout << endl << "Outbreak count = " << new_board.get_outbreak_count() << endl;

    cout << endl << "Try adding one more to get another outbreak!";
    new_board.infect(0,0,1);
    cout << "...success!"<< endl;
    cout << endl << Map::CITIES[0].name << " has " << disease_count[Map::BLUE][0] << " cubes" << endl;
    for(int neighbor: Map::CITIES[0].neighbors){
        cout << Map::CITIES[neighbor].name << " has " << disease_count[Map::BLUE][neighbor] << " BLUE cubes" << endl;
        cout << Map::CITIES[neighbor].name << " has " << disease_count[Map::YELLOW][neighbor] << " YELLOW cubes" << endl;
        cout << Map::CITIES[neighbor].name << " has " << disease_count[Map::BLACK][neighbor] << " BLACK cubes" << endl;
        cout << Map::CITIES[neighbor].name << " has " << disease_count[Map::RED][neighbor] << " RED cubes" << endl;
    }

    cout << endl << "Outbreak count = " << new_board.get_outbreak_count() << endl;

    cout << " ==== CHAIN OUTBREAK TEST - add one more ===== " << endl;
    cout << endl << "Try adding one more to get another outbreak!";
    new_board.infect(0,0,1);
    cout << "...success!"<< endl;

    for(int neighbor: Map::CITIES[0].neighbors){
        cout << Map::CITIES[neighbor].name << " has " << disease_count[Map::BLUE][neighbor] << " BLUE cubes" << endl;
        cout << Map::CITIES[neighbor].name << " has " << disease_count[Map::YELLOW][neighbor] << " YELLOW cubes" << endl;
        cout << Map::CITIES[neighbor].name << " has " << disease_count[Map::BLACK][neighbor] << " BLACK cubes" << endl;
        cout << Map::CITIES[neighbor].name << " has " << disease_count[Map::RED][neighbor] << " RED cubes" << endl;
    }

    cout << endl << "Outbreak count = " << new_board.get_outbreak_count() << "(should be 8) " <<endl;
}