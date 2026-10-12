#pragma once

#include "move.hpp"
#include "board.hpp"

#include <cstdint>
#include <vector>

const int MAX_MOVES = 256;

class Move_Generator {
public:
    Move_Generator(Board& board, U8 previous_movecount) : board(board), bbs(board.bbs), movelist() {
        movelist.reserve(previous_movecount);
    }

    std::vector<Move>& gen_moves();

private:
    Board& board;
    Bitboards& bbs;
    std::vector<Move> movelist;

    void write_moves(ePieceType piece, eSquare from, BB64 to);

    void gen_pawn_moves();
    void gen_bishop_moves();
    void gen_knight_moves();
    void gen_rook_moves();
    void gen_queen_moves();
    void gen_king_moves();
};