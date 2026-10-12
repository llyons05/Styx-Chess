#pragma once

#include <cassert>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <nmmintrin.h>
#include <numeric>
#include <string>

typedef  uint8_t  U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;


enum eColor : U8 {
    WHITE,
    BLACK,
    NUM_COLORS
};


inline eColor operator++(eColor& color, int) {
    eColor temp = color;
    color = static_cast<eColor>((color + 1) % (NUM_COLORS + 1));
    return temp;
}


constexpr eColor operator!(eColor color) {
    switch (color) {
        case WHITE: return BLACK;
        case BLACK: return WHITE;
        default: return NUM_COLORS;
    }
}


constexpr U8 relative_row(eColor c, U8 row) {
    return (c == WHITE) ? 0 + row : 7 - row;
}


enum ePieceType : U8 {
    PAWN,
    BISHOP,
    KNIGHT,
    ROOK,
    QUEEN,
    KING,
    NUM_PIECE_TYPES
};


constexpr ePieceType operator++(ePieceType& piece, int) {
    ePieceType temp = piece;
    piece = static_cast<ePieceType>((piece + 1) % (NUM_PIECE_TYPES + 1));
    return temp;
}


// Small wrapper class for a piece
class Piece {
public:
    eColor color : 4;
    ePieceType pt : 4;

    constexpr Piece() : color(NUM_COLORS), pt(NUM_PIECE_TYPES) {}
    constexpr Piece(eColor color, ePieceType pt) : color(color), pt(pt) {}
    constexpr bool is_null() const { return (color == NUM_COLORS) || (pt == NUM_PIECE_TYPES); }
};


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
    square = static_cast<eSquare>((square + 1) % (NULL_SQUARE));
    return temp;
}

constexpr std::string square_str(eSquare square) {
    int col = square % 8;
    int row = square / 8;
    return "abcdefgh"[col] + std::to_string(row+1);
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
    NUM_FILES
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


enum Direction : U8 {
    NORTH,
    NORTHEAST,
    EAST,
    SOUTHEAST,
    SOUTH,
    SOUTHWEST,
    WEST,
    NORTHWEST,
    NUM_DIRECTIONS
};

constexpr Direction operator!(Direction dir) {
    return Direction((dir + 4) % NUM_DIRECTIONS);
}

constexpr U8 binary_to_square(U64 binary) {
    if (binary == 0) return 0;
    return __builtin_ctzll(binary);
}

constexpr U64 get_time() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

inline std::string get_colored(std::string original, int color_code) {
    return "\033[" + std::to_string(color_code) + "m" + original + "\033[0m";
}

template <class Num_Type>
inline Num_Type get_bounded_num(std::string prompt, Num_Type lower_inclusive = std::numeric_limits<Num_Type>::min(), Num_Type upper_exclusive = std::numeric_limits<Num_Type>::max()) {
    std::cout << prompt;
    Num_Type in;
    std::cin >> in;
    while (std::cin.fail() || (in < lower_inclusive) || (in >= upper_exclusive)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << get_colored("\nInvalid entry\n", 31) << prompt;
        std::cin >> in;
    }
    std::cout << "\n";
    return in;
}