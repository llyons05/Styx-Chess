#pragma once

#include <cstdint>
#include <nmmintrin.h>
#include <cassert>

typedef  uint8_t  U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;


enum eColor {
    WHITE,
    BLACK,
    NULL_COLOR
};

inline eColor operator++(eColor& color, int) {
    eColor temp = color;
    color = static_cast<eColor>((color + 1) % (NULL_COLOR + 1));
    return temp;
}

constexpr eColor operator!(eColor color) {
    switch (color) {
        case WHITE: return BLACK;
        case BLACK: return WHITE;
        default: return NULL_COLOR;
    }
}

enum ePieceType : U8 {
    PAWN,
    BISHOP,
    KNIGHT,
    ROOK,
    QUEEN,
    KING,
    NUM_PIECE_TYPES,
    NULL_PT
};

constexpr ePieceType operator++(ePieceType& piece, int) {
    ePieceType temp = piece;
    piece = static_cast<ePieceType>((piece + 1) % (NULL_PT + 1));
    return temp;
}


enum ePiece : U8 {
    W_PAWN = PAWN,      W_BISHOP, W_KNIGHT, W_ROOK, W_QUEEN, W_KING,
    B_PAWN = PAWN + 8,  B_BISHOP, B_KNIGHT, B_ROOK, B_QUEEN, B_KING,
    NULL_PIECE = 15
};

constexpr ePiece operator&(eColor color, ePieceType pt) {
    return static_cast<ePiece>(pt | ((color == WHITE) ? 0 : 8));
}

constexpr eColor color_of(ePiece piece) {
    assert(piece != NULL_PIECE);
    return static_cast<eColor>(piece >> 3); // 4th bit will encode the color
}

constexpr ePieceType type_of(ePiece piece) {
    return static_cast<ePieceType>(piece & 7); // Lower 3 bits encode piece type
}


enum eSquare : U8 {
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8,
    NULL_SQUARE
};

constexpr eSquare operator++(eSquare& square, int) {
    eSquare temp = square;
    square = static_cast<eSquare>((square + 1) % (NULL_SQUARE + 1));
    return temp;
}


enum File : U8 {
    FILE_A,
    FILE_B,
    FILE_C,
    FILE_D,
    FILE_E,
    FILE_F,
    FILE_G,
    FILE_H,
    FILE_NULL
};


enum eCastle : U8 {
    CASTLE_WK = 1,
    CASTLE_WQ = 2,
    CASTLE_BK = 4,
    CASTLE_BQ = 8
};

enum eSpecialMoveFlags : U8 {
    MFLAG_NORMAL,
    MFLAG_CASTLE,
    MFLAG_EP,
    MFLAG_PROMOTION,
    MFLAG_NULLMOVE
};


enum Direction : int8_t {
    NORTH = 8,
    EAST = 1,
    SOUTH = -NORTH,
    WEST = -EAST,

    NORTH_EAST = NORTH + EAST,
    SOUTH_EAST = SOUTH + EAST,
    SOUTH_WEST = SOUTH + WEST,
    NORTH_WEST = NORTH + WEST
};


constexpr U8 binary_to_square(U64 binary) {
    if (binary == 0) return 0;
    return __builtin_ctzll(binary);
}

U64 get_time();
void print_binary(U64 binary);

constexpr U64 SQUARES[64] = {
1 << A1, 1 << B1, 1 << C1, 1 << D1, 1 << E1, 1 << F1, 1 << G1, 1 << H1,
1 << A2, 1 << B2, 1 << C2, 1 << D2, 1 << E2, 1 << F2, 1 << G2, 1 << H2,
1 << A3, 1 << B3, 1 << C3, 1 << D3, 1 << E3, 1 << F3, 1 << G3, 1 << H3,
1 << A4, 1 << B4, 1 << C4, 1 << D4, 1 << E4, 1 << F4, 1 << G4, (U64)1 << H4,
(U64)1 << A5, (U64)1 << B5, (U64)1 << C5, (U64)1 << D5, (U64)1 << E5, (U64)1 << F5, (U64)1 << G5, (U64)1 << H5,
(U64)1 << A6, (U64)1 << B6, (U64)1 << C6, (U64)1 << D6, (U64)1 << E6, (U64)1 << F6, (U64)1 << G6, (U64)1 << H6,
(U64)1 << A7, (U64)1 << B7, (U64)1 << C7, (U64)1 << D7, (U64)1 << E7, (U64)1 << F7, (U64)1 << G7, (U64)1 << H7,
(U64)1 << A8, (U64)1 << B8, (U64)1 << C8, (U64)1 << D8, (U64)1 << E8, (U64)1 << F8, (U64)1 << G8, (U64)1 << H8
};

const U64 Rank_BB[8] {
    SQUARES[A1] | SQUARES[B1] | SQUARES[C1] | SQUARES[D1] | SQUARES[E1] | SQUARES[F1] | SQUARES[G1] | SQUARES[H1],
    SQUARES[A2] | SQUARES[B2] | SQUARES[C2] | SQUARES[D2] | SQUARES[E2] | SQUARES[F2] | SQUARES[G2] | SQUARES[H2],
    SQUARES[A3] | SQUARES[B3] | SQUARES[C3] | SQUARES[D3] | SQUARES[E3] | SQUARES[F3] | SQUARES[G3] | SQUARES[H3],
    SQUARES[A4] | SQUARES[B4] | SQUARES[C4] | SQUARES[D4] | SQUARES[E4] | SQUARES[F4] | SQUARES[G4] | SQUARES[H4],
    SQUARES[A5] | SQUARES[B5] | SQUARES[C5] | SQUARES[D5] | SQUARES[E5] | SQUARES[F5] | SQUARES[G5] | SQUARES[H5],
    SQUARES[A6] | SQUARES[B6] | SQUARES[C6] | SQUARES[D6] | SQUARES[E6] | SQUARES[F6] | SQUARES[G6] | SQUARES[H6],
    SQUARES[A7] | SQUARES[B7] | SQUARES[C7] | SQUARES[D7] | SQUARES[E7] | SQUARES[F7] | SQUARES[G7] | SQUARES[H7],
    SQUARES[A8] | SQUARES[B8] | SQUARES[C8] | SQUARES[D8] | SQUARES[E8] | SQUARES[F8] | SQUARES[G8] | SQUARES[H8]
};

const U64 File_BB[8] {
    SQUARES[A1] | SQUARES[A2] | SQUARES[A3] | SQUARES[A4] | SQUARES[A5] | SQUARES[A6] | SQUARES[A7] | SQUARES[A8],
    SQUARES[B1] | SQUARES[B2] | SQUARES[B3] | SQUARES[B4] | SQUARES[B5] | SQUARES[B6] | SQUARES[B7] | SQUARES[B8],
    SQUARES[C1] | SQUARES[C2] | SQUARES[C3] | SQUARES[C4] | SQUARES[C5] | SQUARES[C6] | SQUARES[C7] | SQUARES[C8],
    SQUARES[D1] | SQUARES[D2] | SQUARES[D3] | SQUARES[D4] | SQUARES[D5] | SQUARES[D6] | SQUARES[D7] | SQUARES[D8],
    SQUARES[E1] | SQUARES[E2] | SQUARES[E3] | SQUARES[E4] | SQUARES[E5] | SQUARES[E6] | SQUARES[E7] | SQUARES[E8],
    SQUARES[F1] | SQUARES[F2] | SQUARES[F3] | SQUARES[F4] | SQUARES[F5] | SQUARES[F6] | SQUARES[F7] | SQUARES[F8],
    SQUARES[G1] | SQUARES[G2] | SQUARES[G3] | SQUARES[G4] | SQUARES[G5] | SQUARES[G6] | SQUARES[G7] | SQUARES[G8],
    SQUARES[H1] | SQUARES[H2] | SQUARES[H3] | SQUARES[H4] | SQUARES[H5] | SQUARES[H6] | SQUARES[H7] | SQUARES[H8]
};