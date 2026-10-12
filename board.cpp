#include "board.hpp"

#include "utils.hpp"

#include <iostream>

Board::Board() : bbs() {
    
}

void Board::reset() {
    
}

void Board::move(Move& m) {
    bbs.handle_move(m);
}


void Board::print() {
    const char piece_chars[2][8] = {{'P', 'B', 'N', 'R', 'Q', 'K', '-', '-'}, {'p', 'b', 'n', 'r', 'q', 'k', '-', '-'}};
    
    for (int row = 7; row >= 0; row--) {
        std::cout << "   " << (char)('1' + row) << "\t";
        for (int col = 0; col < 8; col++) {
            char print_char = '-';
            Piece piece = bbs.piece_at(static_cast<eSquare>(row*8 + col));
            if (piece.color != NUM_COLORS) {
                print_char = piece_chars[piece.color][piece.pt];
            }
            std::cout << print_char << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n\ta b c d e f g h\n";
    std::cout << ((bbs.side_to_move == WHITE) ? "White" : "Black") << " to move\n";
}