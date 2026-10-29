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


enum eEntry_Type : U8 {
    TT_EXACT,
    TT_LOWERBOUND,
    TT_UPPERBOUND
};


class TT_Entry {
public:
    U64 hash    = 0;
    Score eval  = 0;
    Depth depth = -1;
    eEntry_Type entry_type = TT_EXACT;
    U8 best_move_id = 255;

    TT_Entry() {};

    inline void update(U64 hash, Score eval, Depth depth, U8 best_move_id, eEntry_Type entry_type) {
        this->hash = hash;
        this->eval = eval;
        this->depth = depth;
        this->best_move_id = best_move_id;
        this->entry_type = entry_type;
    }

    inline Score root_relative_eval(int ply) {
        if (abs(eval) >= MIN_WINNING_SCORE) {
            if (eval > 0) 
                return eval - ply;
            return eval + ply;
        }
        return eval;
    }
};


class Transposition_Table {
public:
    U64 size;
    U64 used;
    std::unique_ptr<TT_Entry[]> table;

    Transposition_Table(U64 bytes) :
        size(std::max(1ULL, (bytes / sizeof(TT_Entry)) - 1)),
        used(0),
        table(std::make_unique<TT_Entry[]>(size)) {}

    Transposition_Table& operator=(Transposition_Table& other) {
        if (this != &other) {
            size = other.size;
            table = std::move(other.table);
        }
        return *this;
    }

    constexpr void clear() {
        std::fill_n(table.get(), size, TT_Entry{});
        used = 0;
    }

    constexpr U64 hashfull() {
        return (used*1000) / size;
    }
    
    constexpr TT_Entry& at(U64 hash) {
        return table.get()[hash % size];
    }

    constexpr bool contains(U64 hash) {
        return at(hash).hash == hash;
    }

    inline void store(U64 hash, Score eval, Depth depth, int ply, U8 best_move_id, eEntry_Type entry_type) {
        TT_Entry& entry = at(hash);
        if ((entry.hash == hash) && (entry.depth > depth))
            return;
        
        if (abs(eval) >= MIN_WINNING_SCORE) {
            if (eval > 0) eval += ply;
            else          eval -= ply;
        }
        if (entry.depth == -1) used++;
        entry.update(hash, eval, depth, best_move_id, entry_type);
    }
};