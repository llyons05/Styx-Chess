#include "board.hpp"

#include "utils.hpp"

#include <iostream>

Board::Board() : bbs() {
    
}

void Board::reset() {
    
}


void Board::print() {
    const char piece_chars[2][8] = {{'P', 'B', 'N', 'R', 'Q', 'K', '-', '-'}, {'p', 'b', 'n', 'r', 'q', 'k', '-', '-'}};
    
    for (int row = 7; row >= 0; row--) {
        std::cout << "   " << (char)('1' + row) << "\t";
        for (int col = 0; col < 8; col++) {
            char print_char = '-';
            eSquare square = static_cast<eSquare>(row*8 + col);
            eColor color = bbs.color_at(square);
            if (color != NULL_COLOR) {
                ePieceType piece = bbs.piece_at(color, square);
                print_char = piece_chars[color][piece];
            }
            std::cout << print_char << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n\ta b c d e f g h\n";
    std::cout << ((bbs.side_to_move == WHITE) ? "White" : "Black") << " to move\n";
}