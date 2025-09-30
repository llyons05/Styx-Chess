#pragma once

#include "move.hpp"
#include "board.hpp"

#include <cstdint>
#include <vector>

const int MAX_LEGAL_MOVES = 218;

class Move_Generator {
public:
    Move_Generator(const Board& board, U8 previous_movecount) : board(board) {
        movelist.reserve(previous_movecount);
    }

    std::vector<Move>& gen_moves(U8 expected_best);

private:
    const Board& board;
    std::vector<Move> movelist;

    void gen_pawn_moves();
    void gen_bishop_moves();
    void gen_knight_moves();
    void gen_rook_moves();
    void gen_queen_moves();
    void gen_king_moves();
};