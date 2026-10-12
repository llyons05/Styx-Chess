#include "utils.hpp"
#include "board.hpp"
#include "movegen.hpp"

#include <iostream>
#include <string>

int main() {
    Board board;
    board.print();

    while (true) {
        Move_Generator mg(board);
        for (int i = 0; i < mg.movelist.size(); i++) {
            std::cout << i << ": " << mg.movelist[i] << ", ";
        }
        std::cout << "\n";
        int move_index = get_bounded_num<int>("Type move id: ", 0, mg.movelist.size());
        Move m = mg.movelist[move_index];
        board.move(m);
        board.print();
        std::string line;
        std::getline(std::cin, line);
    }
}