#pragma once

#include <cstdint>
#include <nmmintrin.h>

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

inline eColor operator!(eColor color) {
    switch (color) {
        case WHITE: return BLACK;
        case BLACK: return WHITE;
        default: return NULL_COLOR;
    }
}

enum ePiece : U8 {
    PAWN,
    BISHOP,
    KNIGHT,
    ROOK,
    QUEEN,
    KING,
    NUM_PIECE_TYPES,
    NULL_PIECE
};

inline ePiece operator++(ePiece& piece, int) {
    ePiece temp = piece;
    piece = static_cast<ePiece>((piece + 1) % (NULL_PIECE + 1));
    return temp;
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

inline eSquare operator++(eSquare& square, int) {
    eSquare temp = square;
    square = static_cast<eSquare>((square + 1) % (NULL_SQUARE + 1));
    return temp;
}

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

inline U8 count_bits(U64 bits) {
    #ifdef __GNUC__
	   return __builtin_popcountll(bits);
   #else
	   return _mm_popcnt_u64(bits);
   #endif
}

inline U8 binary_to_square(U64 binary) {
    if (binary == 0) return 0;
    return __builtin_ctzll(binary);
}

U64 get_time();
void print_binary(U64 binary);

const U64 SQUARES[64] = {
1 << A1, 1 << B1, 1 << C1, 1 << D1, 1 << E1, 1 << F1, 1 << G1, 1 << H1,
1 << A2, 1 << B2, 1 << C2, 1 << D2, 1 << E2, 1 << F2, 1 << G2, 1 << H2,
1 << A3, 1 << B3, 1 << C3, 1 << D3, 1 << E3, 1 << F3, 1 << G3, 1 << H3,
1 << A4, 1 << B4, 1 << C4, 1 << D4, 1 << E4, 1 << F4, 1 << G4, (U64)1 << H4,
(U64)1 << A5, (U64)1 << B5, (U64)1 << C5, (U64)1 << D5, (U64)1 << E5, (U64)1 << F5, (U64)1 << G5, (U64)1 << H5,
(U64)1 << A6, (U64)1 << B6, (U64)1 << C6, (U64)1 << D6, (U64)1 << E6, (U64)1 << F6, (U64)1 << G6, (U64)1 << H6,
(U64)1 << A7, (U64)1 << B7, (U64)1 << C7, (U64)1 << D7, (U64)1 << E7, (U64)1 << F7, (U64)1 << G7, (U64)1 << H7,
(U64)1 << A8, (U64)1 << B8, (U64)1 << C8, (U64)1 << D8, (U64)1 << E8, (U64)1 << F8, (U64)1 << G8, (U64)1 << H8
};