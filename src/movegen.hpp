#pragma once

#include "board.hpp"
#include "move.hpp"

#include <array>
#include <cstdint>
#include <vector>

const int MAX_MOVES = 218;

class Move_Generator {
public:
    std::vector<Move> movelist;
    BB64 destinations;

    Move_Generator(const Board& board) : movelist(), destinations(0), board(board), bbs(board.bbs) {
        // movelist.reserve(previous_movecount);
        if (board.stm() == WHITE)
            gen_moves<WHITE>();
        else
            gen_moves<BLACK>();
    }

    Move& next_move(int start) {
        int move_index = start;
        int best_score = movelist[move_index].score;

        for (size_t i = move_index+1; i < movelist.size(); i++) {
            if (movelist[i].score > best_score) {
                best_score = movelist[i].score;
                move_index = i;
            }
        }

        std::swap(movelist[start], movelist[move_index]);
        return movelist[start];
    }

    constexpr Move& operator[](U8 index) {
        assert(index < movelist.size());
        return movelist[index];
    }

    static bool is_attacked(const Board& board, eColor attackers, BB64 bb);
    static BB64 attacks(const Board& board, eColor attackers);

private:
    const Board& board;
    const Bitboards& bbs;

    template<eColor C>
    std::vector<Move>& gen_moves();

    void write_moves(ePieceType piece_from, ePieceType piece_to, eSquare from, BB64 to, eSpecialMoveFlags flag);

    template<eColor C>
    void gen_pawn_moves();

    void gen_bishop_moves();
    void gen_knight_moves();
    void gen_rook_moves();
    void gen_queen_moves();
    void gen_king_moves();

    template<Direction D>
    static BB64 sliding_threats(BB64 pieces, const Board& board, eColor color);

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

template <ePieceType pt>
class Magic_Table {
private:
    static_assert(pt == BISHOP || pt == ROOK);
    static const size_t num_attacks = pt == BISHOP ? 512 : 4096;
    BB64 attacks[num_attacks];
    BB64 mask;
    U64 magic;
    int shift;

    static BB64 rel_occupancy_bits(eSquare s);
    static BB64 attack_bb(eSquare s, BB64 occupied);
    static BB64 blocker_index_mask(BB64 occupancy_bits, int blocker_index);

    size_t blockers_to_index(BB64 blockers);

public:
    Magic_Table() : attacks{}, mask(0), magic(0), shift(0) {}
    
    void init(eSquare s);
    BB64 moves(BB64 occupied) {
        return attacks[blockers_to_index(occupied & mask)];
    }
};

void init_magics();

constexpr std::array<BB64, 64> knight_moves = init_knight_moves();
constexpr std::array<BB64, 64> king_moves = init_king_moves();

extern std::array<Magic_Table<BISHOP>, 64> bishop_magics;
extern std::array<Magic_Table<ROOK>, 64> rook_magics;