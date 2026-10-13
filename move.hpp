#pragma once

#include "utils.hpp"

class Move {
public:
    U8 id; // The index at which this move was generated. This is also used for promotion piece stuff.

    eSquare from : 7; // The index the move started on
    eSquare to : 7; // The index the move ended on

    ePieceType piece_from : 3;
    ePieceType piece_to : 3;
    ePieceType piece_captured : 3;
    
    eSpecialMoveFlags move_flag : 3; // Move flags for this move (castle, en passant, etc)

    Move() : id(0),
             from(A1),
             to(A1),
             piece_from(NUM_PIECE_TYPES),
             piece_to(NUM_PIECE_TYPES),
             piece_captured(NUM_PIECE_TYPES),
             move_flag(MFLAG_NORMAL) {}
    
    Move(eSquare from,
         eSquare to,
         ePieceType piece_from,
         ePieceType piece_to,
         ePieceType piece_captured,
         eSpecialMoveFlags move_flag,
         U8 id)
         :
         id(id),
         from(from),
         to(to),
         piece_from(piece_from),
         piece_to(piece_to),
         piece_captured(piece_captured),
         move_flag(move_flag) {}

    inline bool is_capture() const {
        return piece_captured != NUM_PIECE_TYPES;
    }

    inline bool is_nullmove() const {
        return move_flag & MFLAG_NULLMOVE;
    }
    
    friend std::ostream& operator<<(std::ostream& os, const Move& m) {
        os << " BNRQK"[m.piece_from] << square_str(m.from) << (m.is_capture() ? 'x' : '-') << square_str(m.to);
        return os;
    }
};