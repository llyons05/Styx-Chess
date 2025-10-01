#pragma once

#include "utils.hpp"
#include "move.hpp"

#include <cstdint>

// Wrapper class for a 64-bit bitboard
class BB64 {
public:
    U64 bb;

    inline BB64() : bb(0) {};
    inline BB64(U64 bb) : bb(bb) {}
    
    constexpr void zero(eSquare square) {
        bb &= ~SQUARES[square];
    }
    
    constexpr void fill(eSquare square) {
        bb |= SQUARES[square];
    }

    constexpr bool filled(eSquare square) const {
        return bb & SQUARES[square];
    }
    
    inline U8 bitcount() const {
        return count_bits(bb);
    }

    template<Direction D>
    constexpr BB64 shift() {
        return BB64(
          D == NORTH         ? bb << 8
        : D == SOUTH         ? bb >> 8
        : D == NORTH + NORTH ? bb << 16
        : D == SOUTH + SOUTH ? bb >> 16
        : D == EAST          ? (bb & ~File_BB[FILE_H]) << 1
        : D == WEST          ? (bb & ~File_BB[FILE_A]) >> 1
        : D == NORTH_EAST    ? (bb & ~File_BB[FILE_H]) << 9
        : D == NORTH_WEST    ? (bb & ~File_BB[FILE_A]) << 7
        : D == SOUTH_EAST    ? (bb & ~File_BB[FILE_H]) >> 7
        : D == SOUTH_WEST    ? (bb & ~File_BB[FILE_A]) >> 9
                            : 0);
    }
    
    inline BB64 operator&(const BB64& other) const {
        return BB64(bb & other.bb);
    }
    inline BB64 operator&=(const BB64 other) {
        bb &= other.bb;
        return *this;
    }
    
    inline BB64 operator|(const BB64& other) const {
        return BB64(bb | other.bb);
    }
    inline BB64 operator|=(const BB64 other) {
        bb |= other.bb;
        return *this;
    }
    
    inline BB64 operator<<(U8 shift) const {
        return BB64(bb << shift);
    }
    
    inline BB64 operator>>(U8 shift) const {
        return BB64(bb >> shift);
    }
    
    inline BB64& operator=(U64 new_bb) {
        bb = new_bb;
        return *this;
    }
    
    inline operator bool() const {
        return bb;
    }

    explicit inline operator U64() const {
        return bb;
    }
};


class Bitboards {
public:
    BB64 piece_bbs[2][NUM_PIECE_TYPES];
    BB64 color_bbs[2];
    eColor side_to_move;
    U8 castling_rights;
    U64 hash_key;

    Bitboards();
    Bitboards(const Bitboards& other);

    void handle_move(const Move& move);

    Piece piece_at(eSquare square) const;
    ePieceType piece_at(eColor color, eSquare square) const;
    eColor color_at(eSquare square) const;

private:
    inline void pickup_piece(eSquare square, Piece piece);
    inline Piece pickup_piece(eSquare square, eColor color);
    inline Piece put_piece(eSquare square, Piece piece);
};


const BB64 STARTING_PIECE_POSITIONS[2][6] = {
    {
        SQUARES[A2] | SQUARES[B2] | SQUARES[C2] | SQUARES[D2] | SQUARES[E2] | SQUARES[F2] | SQUARES[G2] | SQUARES[H2],
        SQUARES[C1] | SQUARES[F1],
        SQUARES[B1] | SQUARES[G1],
        SQUARES[A1] | SQUARES[H1],
        SQUARES[D1],
        SQUARES[E1]
    },
    {
        SQUARES[A7] | SQUARES[B7] | SQUARES[C7] | SQUARES[D7] | SQUARES[E7] | SQUARES[F7] | SQUARES[G7] | SQUARES[H7],
        SQUARES[C8] | SQUARES[F8],
        SQUARES[B8] | SQUARES[G8],
        SQUARES[A8] | SQUARES[H8],
        SQUARES[D8],
        SQUARES[E8]
    }
};

const BB64 STARTING_COLOR_POSITIONS[2] = {
    STARTING_PIECE_POSITIONS[WHITE][PAWN]   | STARTING_PIECE_POSITIONS[WHITE][BISHOP] | 
    STARTING_PIECE_POSITIONS[WHITE][KNIGHT] | STARTING_PIECE_POSITIONS[WHITE][ROOK]   |
    STARTING_PIECE_POSITIONS[WHITE][QUEEN]  | STARTING_PIECE_POSITIONS[WHITE][KING],

    STARTING_PIECE_POSITIONS[BLACK][PAWN]   | STARTING_PIECE_POSITIONS[BLACK][BISHOP] | 
    STARTING_PIECE_POSITIONS[BLACK][KNIGHT] | STARTING_PIECE_POSITIONS[BLACK][ROOK]   |
    STARTING_PIECE_POSITIONS[BLACK][QUEEN]  | STARTING_PIECE_POSITIONS[BLACK][KING]
};