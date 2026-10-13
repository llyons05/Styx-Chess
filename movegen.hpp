#pragma once

#include "move.hpp"
#include "board.hpp"

#include <array>
#include <cstdint>
#include <vector>

const int MAX_MOVES = 218;

class Move_Generator {
public:
    Move_Generator(const Board& board) : movelist(), destinations(0), board(board), bbs(board.bbs) {
        // movelist.reserve(previous_movecount);
        if (board.stm() == WHITE)
            gen_moves<WHITE>();
        else
            gen_moves<BLACK>();
    }
    std::vector<Move> movelist;
    BB64 destinations;

    static bool is_attacked(const Board& board, eColor attackers, eSquare square);
private:
    const Board& board;
    const Bitboards& bbs;

    template<eColor C>
    std::vector<Move>& gen_moves();

    void write_moves(ePieceType piece, eSquare from, BB64 to);

    template<eColor C>
    void gen_pawn_moves();

    void gen_bishop_moves();
    void gen_knight_moves();
    void gen_rook_moves();
    void gen_queen_moves();
    void gen_king_moves();

    template<Direction D>
    static BB64 valid_sliding_moves(BB64 pieces, const Board& board, eColor color);

    template <eColor C>
    static BB64 pawn_attacks(const Board& board);
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