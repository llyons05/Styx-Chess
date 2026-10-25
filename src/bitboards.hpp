#pragma once

#include "move.hpp"
#include "utils.hpp"
#include "transposition.hpp"

#include <cstdint>
#include <bit>
#include <iostream>
#include <string>

constexpr U64 FILLED_COLS[8] = {72340172838076673ULL, 217020518514230019ULL, 506381209866536711ULL, 1085102592571150095ULL, 2242545357980376863ULL, 4557430888798830399ULL, 9187201950435737471ULL, 18446744073709551615ULL};

// Wrapper class for a 64-bit bitboard
class BB64 {
public:
    U64 bb;

    constexpr BB64() : bb(0) {};
    constexpr BB64(U64 bb) : bb(bb) {}
    constexpr BB64(eSquare s) : bb(1ULL << U64(s)) {}
    constexpr BB64(const BB64& bb) : bb(bb.bb) {}
    
    void print() const;

    constexpr bool empty() {
        return bb == 0;
    }
    
    constexpr void zero(eSquare square) {
        bb &= ~(1ULL << square);
    }
    
    constexpr void fill(eSquare square) {
        bb |= (1ULL << square);
    }

    constexpr bool filled(eSquare square) const {
        return bb & (1ULL << square);
    }
    
    constexpr U8 bitcount() const {
        return std::__popcount(bb);
    }

    constexpr eSquare lsb() const {
        assert(bb); // Cannot be 0
        return eSquare(std::__countr_zero(bb));
    }

    constexpr eSquare pop() {
        const eSquare s = lsb();
        bb &= bb - 1;
        return s;
    }

    template<Direction D, U8 Shifts = 1>
    constexpr BB64 shift() const {
        static_assert((D < NUM_DIRECTIONS) && (Shifts < 8) && (Shifts > 0));
        constexpr BB64 shift_mask = can_dir_shift_bb<D, Shifts>();
        return D == NORTH ? bb << 8*Shifts
            : D == SOUTH ? bb >> 8*Shifts
            : D == EAST  ? (shift_mask & bb) << 1*Shifts
            : D == WEST  ? (shift_mask & bb) >> 1*Shifts

            : D == NORTHEAST ? (shift_mask & bb) << 9*Shifts
            : D == SOUTHEAST ? (shift_mask & bb) >> 7*Shifts
            : D == SOUTHWEST ? (shift_mask & bb) >> 9*Shifts
            :/*D == NORTHWEST*/(shift_mask & bb) << 7*Shifts;
    }

    template<Direction D>
    constexpr BB64 smear() const {
        static_assert(D < NUM_DIRECTIONS);
        BB64 n = bb;
        n |= n.shift<D,1>();
        n |= n.shift<D,2>();
        n |= n.shift<D,4>();
        return n;
    }
    
    constexpr BB64 operator&(const BB64& other) const {
        return BB64(bb & other.bb);
    }
    constexpr BB64 operator&=(const BB64 other) {
        bb &= other.bb;
        return *this;
    }
    
    constexpr BB64 operator|(const BB64& other) const {
        return BB64(bb | other.bb);
    }
    constexpr BB64 operator|=(const BB64 other) {
        bb |= other.bb;
        return *this;
    }
    
    constexpr BB64 operator<<(U8 shift) const {
        return BB64(bb << shift);
    }
    
    constexpr BB64 operator>>(U8 shift) const {
        return BB64(bb >> shift);
    }
    
    constexpr BB64& operator=(U64 new_bb) {
        bb = new_bb;
        return *this;
    }

    constexpr BB64 operator~() const {
        return BB64(~bb);
    }

    constexpr BB64 operator==(const BB64& other) const {
        return bb == other.bb;
    }

    constexpr BB64 operator!=(const BB64& other) const {
        return bb != other.bb;
    }

    constexpr bool operator==(U64 n) const {
        return bb == n;
    }

    constexpr bool operator!=(U64 n) const {
        return bb != n;
    }

    constexpr explicit operator bool() const {
        return bb;
    }

    explicit constexpr operator U64() const {
        return bb;
    }
    private:
    template<Direction D, U8 Shifts>
    static constexpr BB64 can_dir_shift_bb() {
        static_assert((D < NUM_DIRECTIONS) && (Shifts < 8) && (Shifts > 0));
        return D == NORTH ? ~0ULL
             : D == SOUTH ? ~0ULL
             : D == EAST  ? FILLED_COLS[7-Shifts]
             : D == WEST  ? FILLED_COLS[7-Shifts] << Shifts

             : D == NORTHEAST ? FILLED_COLS[7-Shifts]
             : D == SOUTHEAST ? FILLED_COLS[7-Shifts]
             : D == SOUTHWEST ? FILLED_COLS[7-Shifts] << Shifts
             :/*D == NORTHWEST*/FILLED_COLS[7-Shifts] << Shifts;
    }
};

class BB_State_Info {
public:
    U64 hash_key = 0;
    U8 castling_rights = 0;
    eSquare ep_square = NULL_SQUARE;
    U8 fifty_move_rule = 0;
    BB_State_Info() = default;
    BB_State_Info(U64 hash_key, U8 castling_rights, eSquare ep_square, U8 fifty_move_rule) : hash_key(hash_key), castling_rights(castling_rights), ep_square(ep_square), fifty_move_rule(fifty_move_rule) {}
};

class Bitboards {
public:
    BB64 piece_bbs[NUM_COLORS][NUM_PIECE_TYPES]{0};
    U8 piece_counts[NUM_COLORS][NUM_PIECE_TYPES]{0};
    Piece pieces[64]{};
    BB64 color_bbs[NUM_COLORS]{0};
    Score material[NUM_COLORS]{0};
    eColor stm;
    U8 castling_rights;
    eSquare ep_square;
    U8 fifty_move_rule;
    U64 hash_key = 0;

    Bitboards();
    Bitboards(std::string fen);
    Bitboards(const Bitboards& other);

    Piece handle_move(const Move& move);
    void undo_move(const Move& move, const BB_State_Info& state_info);

    Piece piece_at(eSquare square) const;
    eColor color_at(eSquare square) const;
    bool has_mating_material(eColor color) const;

    BB_State_Info state_info() const {
        return BB_State_Info(hash_key, castling_rights, ep_square, fifty_move_rule);
    }

    bool occupied(BB64 b) const {
        return !(all() & b).empty();
    }

    bool empty(BB64 b) const {
        return (all() & b).empty();
    }

    BB64 all() const {
        return color_bbs[WHITE] | color_bbs[BLACK];
    }

    const BB64& stm_bb() const {
        return color_bbs[stm];
    }

    const BB64& stm_bb(ePieceType pt) const {
        return piece_bbs[stm][pt];
    }

    const BB64& opp_bb() const {
        return color_bbs[!stm];
    }

    const BB64& opp_bb(ePieceType pt) const {
        return piece_bbs[!stm][pt];
    }

    const BB64& operator[](eColor color) const {
        return color_bbs[color];
    }

    const BB64& operator[](eColor color, ePieceType piece) const {
        return piece_bbs[color][piece];
    }

    eSquare king_sq(eColor color) const {
        assert(color < NUM_COLORS);
        return piece_bbs[color][KING].lsb();
    }

private:
    inline Piece pickup_piece(eSquare square);
    inline Piece put_piece(eSquare square, Piece piece);
    void update_castling_rights(const Move& m);
    void update_ep_square(const Move& m);
    void change_turn();
};

constexpr BB64 ROWS[8] = {255ULL, 255ULL << 8, 255ULL << 16, 255ULL << 24, 255ULL << 32, 255ULL << 40, 255ULL << 48, 255ULL << 56};
constexpr BB64 COLS[8] = {72340172838076673ULL, 72340172838076673ULL << 1, 72340172838076673ULL << 2, 72340172838076673ULL << 3, 72340172838076673ULL << 4, 72340172838076673ULL << 5, 72340172838076673ULL << 6, 72340172838076673ULL << 7};

const BB64 STARTING_PIECE_POSITIONS[NUM_COLORS][NUM_PIECE_TYPES] = {
    {
        BB64(A2) | BB64(B2) | BB64(C2) | BB64(D2) | BB64(E2) | BB64(F2) | BB64(G2) | BB64(H2),
        BB64(C1) | BB64(F1),
        BB64(B1) | BB64(G1),
        BB64(A1) | BB64(H1),
        BB64(D1),
        BB64(E1)
    },
    {
        BB64(A7) | BB64(B7) | BB64(C7) | BB64(D7) | BB64(E7) | BB64(F7) | BB64(G7) | BB64(H7),
        BB64(C8) | BB64(F8),
        BB64(B8) | BB64(G8),
        BB64(A8) | BB64(H8),
        BB64(D8),
        BB64(E8)
    }
};

const BB64 STARTING_COLOR_POSITIONS[NUM_COLORS] = {
    STARTING_PIECE_POSITIONS[WHITE][PAWN]   | STARTING_PIECE_POSITIONS[WHITE][BISHOP] | 
    STARTING_PIECE_POSITIONS[WHITE][KNIGHT] | STARTING_PIECE_POSITIONS[WHITE][ROOK]   |
    STARTING_PIECE_POSITIONS[WHITE][QUEEN]  | STARTING_PIECE_POSITIONS[WHITE][KING],

    STARTING_PIECE_POSITIONS[BLACK][PAWN]   | STARTING_PIECE_POSITIONS[BLACK][BISHOP] | 
    STARTING_PIECE_POSITIONS[BLACK][KNIGHT] | STARTING_PIECE_POSITIONS[BLACK][ROOK]   |
    STARTING_PIECE_POSITIONS[BLACK][QUEEN]  | STARTING_PIECE_POSITIONS[BLACK][KING]
};