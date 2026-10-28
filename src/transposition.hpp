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


class TT_Entry_Bucket {
public:
    TT_Entry entries[2]{};
    TT_Entry_Bucket() {}

    inline void store(U64 hash, Score eval, Depth depth, int ply, U8 best_move_id, eEntry_Type entry_type) {
        TT_Entry& replaced_entry = entries[0];
        if (depth <= replaced_entry.depth) {
            if (hash == replaced_entry.hash)
                return;
            replaced_entry = entries[1];
        }
        if ((hash == replaced_entry.hash) && (depth <= replaced_entry.depth))
            return;
        
        if (abs(eval) >= MIN_WINNING_SCORE) {
            if (eval > 0) eval += ply;
            else          eval -= ply;
        }

        replaced_entry.update(hash, eval, depth, best_move_id, entry_type);
    }

    constexpr bool contains(U64 hash) const {
        return (entries[0].hash == hash) || (entries[1].hash == hash);
    }

    constexpr TT_Entry& entry(U64 hash) {
        if (entries[0].hash == hash)
            return entries[0];
        return entries[1];
    }
};


class Transposition_Table {
public:
    U64 size;
    std::shared_ptr<TT_Entry_Bucket[]> table;

    Transposition_Table(U64 bytes) :
        size((bytes / sizeof(TT_Entry_Bucket)) - 1),
        table(std::make_unique<TT_Entry_Bucket[]>(size)) {}

    Transposition_Table& operator=(const Transposition_Table& other) {
        if (this != &other) {
            size = other.size;
            table = std::move(other.table);
        }
        return *this;
    }
    
    constexpr TT_Entry_Bucket& at(U64 hash) {
        return table.get()[hash % size];
    }

    constexpr TT_Entry& entry(U64 hash) {
        return at(hash).entry(hash);
    }

    constexpr bool contains(U64 hash) {
        return at(hash).contains(hash);
    }

    inline void store(U64 hash, Score eval, Depth depth, int ply, U8 best_move_id, eEntry_Type entry_type) {
        TT_Entry_Bucket& bucket = at(hash);
        bucket.store(hash, eval, depth, ply, best_move_id, entry_type);
    }
};