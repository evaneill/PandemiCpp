#include <iostream>

#include "../game_files/Board.h"

using namespace std;

int main(){
    srand(time(NULL));
    rand();
    cout << "About to make a board ..." << endl;
    Board::Board new_board = Board::Board({0,2,3},4);
    cout << "... done" << endl;

    cout << "About to set the board up..." << endl;
    new_board.setup();
    cout << "... success!" << endl;
}