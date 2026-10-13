#include "utils.hpp"
#include "board.hpp"
#include "movegen.hpp"

#include <iostream>
#include <string>
#include <vector>

int main() {
    Board board;
    board.print();

    while (true) {
        Move_Generator mg(board);
        if (mg.movelist.empty()) {
            std::cout << "Game Over!\n";
            break;
        }
        eSquare square = NULL_SQUARE;
        std::vector<Move> legal_moves;
        while (legal_moves.empty()) {
            legal_moves.clear();
            square = get_valid_square();
            for (Move& move : mg.movelist) {
                if (move.to == square)
                    legal_moves.push_back(move);
            }
            if (legal_moves.empty())
                std::cout << get_colored("No moves to " + square_str(square) + "\n", 31);
        }
        int move_index = 0;
        if (legal_moves.size() > 1) {
            for (size_t i = 0; i < legal_moves.size(); i++) {
                std::cout << i << ": " << legal_moves[i] << ", ";
            }
            std::cout << "\n";
            move_index = get_bounded_num<int>("Type move id: ", 0, legal_moves.size());
        }
        Move m = legal_moves[move_index];
        board.move(m);
        board.print();
        std::string line;
        std::getline(std::cin, line);
    }
}