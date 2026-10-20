#pragma once

#include "utils.hpp"

class Zobrist {
private:
    U64 piecesquare[2][6][64];
    U64 color_hash;
    U64 castling_hash[16];
    U64 ep_hash[65];

public:
    Zobrist() {
        for (eColor c = WHITE; c < NUM_COLORS; c++) {
            for (ePieceType pt = PAWN; pt < NUM_PIECE_TYPES; pt++) {
                for (eSquare s = A1; s < NULL_SQUARE; s++) {
                    piecesquare[c][pt][s] = rng.rand();
                }
            }
        }
        std::fill_n(&piecesquare[WHITE][PAWN][A8], 8, 0);
        std::fill_n(&piecesquare[BLACK][PAWN][A1], 8, 0);
        color_hash = rng.rand();
        for (int i = 0; i <= 15; i++)
            castling_hash[i] = rng.rand();
        for (int s = A1; s < NULL_SQUARE; s++)
            ep_hash[s] = rng.rand();
        ep_hash[NULL_SQUARE] = 0;
    }

    constexpr U64 operator[](eColor c, ePieceType pt, eSquare s) {
        return piecesquare[c][pt][s];
    }

    constexpr U64 color() {
        return color_hash;
    }

    constexpr U64 castling(U8 castling_rights) {
        return castling_hash[castling_rights];
    }

    constexpr U64 ep(eSquare s) {
        return ep_hash[s];
    }
} extern zobrist;