#pragma once

#include "move.hpp"
#include "board.hpp"

#include <cstdint>
#include <vector>

const int MAX_MOVES = 256;

class Move_Generator {
public:
    Move_Generator(Board& board) : movelist(), board(board), bbs(board.bbs) {
        // movelist.reserve(previous_movecount);
        if (board.stm() == WHITE)
            gen_moves<WHITE>();
        else
            gen_moves<BLACK>();
    }
    std::vector<Move> movelist;

private:
    Board& board;
    Bitboards& bbs;

    template<eColor C>
    std::vector<Move>& gen_moves();

    void write_moves(ePieceType piece, eSquare from, BB64 to);

    template<eColor C>
    void gen_pawn_moves();
    template<eColor C>
    void gen_bishop_moves();
    template<eColor C>
    void gen_knight_moves();
    template<eColor C>
    void gen_rook_moves();
    template<eColor C>
    void gen_queen_moves();
    template<eColor C>
    void gen_king_moves();
};