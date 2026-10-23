#pragma once

#include <cassert>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <nmmintrin.h>
#include <numeric>
#include <string>

#define STRINGIFY(x) TO_STRING(x)
#define TO_STRING(x) #x

typedef  uint8_t  U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;

typedef std::chrono::milliseconds::rep TimePoint;
typedef int Score;
typedef int Depth;

constexpr std::string version_str = STRINGIFY(VERSION_STR);
constexpr TimePoint INF_TIME = std::numeric_limits<TimePoint>::max();

constexpr Depth MAX_PLY = 100;
constexpr int MAX_GAME_LENGTH = 1024;
constexpr Score MAX_SCORE = 100000;
constexpr Score MIN_SCORE = -MAX_SCORE;
constexpr Score INF_SCORE = MAX_SCORE + 1;
constexpr Score MIN_WINNING_SCORE = MAX_SCORE - MAX_PLY;

// From stockfish
class PRNG {

    U64 s;

    U64 rand64() {
        s ^= s >> 12, s ^= s << 25, s ^= s >> 27;
        return s * 2685821657736338717LL;
    }

    public:
    PRNG(U64 seed) :
        s(seed) {
        assert(seed);
    }

    template<typename T = U64>
    T rand() {
        return T(rand64());
    }
} extern rng;

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

struct Time_Info {
    TimePoint movetime = INF_TIME;
    TimePoint remaining[NUM_COLORS]{0};
    TimePoint inc[NUM_COLORS]{0};
    int moves_to_go = 0;
};


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
    constexpr Piece(char c) {
        color = isupper(c) ? WHITE : BLACK;
        switch(tolower(c)) {
            case 'p':
                pt = PAWN;
                break;
            case 'b':
                pt = BISHOP;
                break;
            case 'n':
                pt = KNIGHT;
                break;
            case 'r':
                pt = ROOK;
                break;
            case 'q':
                pt = QUEEN;
                break;
            case 'k':
                pt = KING;
                break;
            default:
                pt = NUM_PIECE_TYPES;
                color = NUM_COLORS;
        }
    }
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
    square = static_cast<eSquare>((square + 1) % (NULL_SQUARE + 1));
    return temp;
}

constexpr std::string square_str(eSquare square) {
    int col = square % 8;
    int row = square / 8;
    return "abcdefgh"[col] + std::to_string(row+1);
}

constexpr eSquare str_to_square(std::string str) {
    if (str.size() != 2)
        return NULL_SQUARE;
    int col = str[0] - 'a';
    int row = str[1] - '1';
    if (row < 0 || row > 7)
        return NULL_SQUARE;
    if (col < 0 || col > 7)
        return NULL_SQUARE;
    return eSquare(row*8 + col);
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
    MFLAG_PROMO,
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

inline TimePoint get_time() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
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

inline eSquare get_valid_square() {
    std::string str;
    std::cout << "Input the square to move to: ";
    std::cin >> str;
    while (std::cin.fail() || (str.length() > 2) || (str_to_square(str) == NULL_SQUARE)) {
        if (str == "undo")
            return NULL_SQUARE;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << get_colored("Nonexistent Square\n", 31) << "Input the square to move to: ";
        str.clear();
        std::cin >> str;
    }
    return str_to_square(str);
}