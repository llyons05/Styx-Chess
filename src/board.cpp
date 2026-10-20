#include "board.hpp"

#include "utils.hpp"

#include <iostream>

Piece Board::move(const Move& m) {
    Piece result = bbs.handle_move(m);
    rep_stack[(++stack_index) % MAX_GAME_LENGTH] = bbs.hash_key;
    return result;
}

void Board::undo(const Move& m, const BB_State_Info& state_info) {
    bbs.undo_move(m, state_info);
    stack_index--;
}

bool Board::is_draw() {
    if (bbs.fifty_move_rule >= 100)
        return true;
    
    if (!bbs.has_mating_material(WHITE) && !bbs.has_mating_material(BLACK))
        return true;

    int curr = stack_index;
    int seen = 0;
    int lowest = stack_index - bbs.fifty_move_rule;
    while ((curr >= lowest) && (curr > 1) && (seen < 2)) {
        curr -= 2;
        if (rep_stack[curr % MAX_GAME_LENGTH ] == bbs.hash_key)
            seen++;
    }
    return seen >= 2;
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
    std::cout << ((bbs.stm == WHITE) ? "White" : "Black") << " to move\t";
    std::cout << (bbs.castling_rights == 0 ? "-":"") 
              << (bbs.castling_rights & CASTLE_WK ? "K":"")
              << (bbs.castling_rights & CASTLE_WQ ? "Q":"")
              << (bbs.castling_rights & CASTLE_BK ? "k":"")
              << (bbs.castling_rights & CASTLE_BQ ? "q":"") << "\t";
    std::cout << (bbs.ep_square == NULL_SQUARE ? "-" : square_str(bbs.ep_square)) << "\n";
}