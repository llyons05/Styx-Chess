#pragma once

#include "move.hpp"
#include "board.hpp"

#include <array>
#include <cstdint>
#include <vector>

const int MAX_MOVES = 218;

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


constexpr std::array<BB64, 64> init_knight_moves() {
    std::array<BB64, 64> result;
    for (int s = 0; s < NULL_SQUARE; s++) {
        BB64 knight = BB64(eSquare(s));
        BB64 valid_moves = knight.shift<NORTH, 2>().shift<EAST>();
        valid_moves |= knight.shift<NORTH, 2>().shift<WEST>();

        valid_moves |= knight.shift<SOUTH, 2>().shift<EAST>();
        valid_moves |= knight.shift<SOUTH, 2>().shift<WEST>();

        valid_moves |= knight.shift<EAST, 2>().shift<NORTH>();
        valid_moves |= knight.shift<EAST, 2>().shift<SOUTH>();
        
        valid_moves |= knight.shift<WEST, 2>().shift<NORTH>();
        valid_moves |= knight.shift<WEST, 2>().shift<SOUTH>();

        result[s] = valid_moves;
    }
    return result;
}

constexpr std::array<BB64, 64> init_king_moves() {
    std::array<BB64, 64> result;
    for (int s = 0; s < NULL_SQUARE; s++) {
        BB64 king = BB64(eSquare(s));
        BB64 valid_moves = king.shift<NORTH>();
        valid_moves |= king.shift<NORTHEAST>();
        valid_moves |= king.shift<EAST>();
        valid_moves |= king.shift<SOUTHEAST>();
        valid_moves |= king.shift<SOUTH>();
        valid_moves |= king.shift<SOUTHWEST>();
        valid_moves |= king.shift<WEST>();
        valid_moves |= king.shift<NORTHWEST>();
        result[s] = valid_moves;
    }
    return result;
}

constexpr std::array<BB64, 64> knight_moves = init_knight_moves();
constexpr std::array<BB64, 64> king_moves = init_king_moves();