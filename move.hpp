#pragma once

#include "utils.hpp"

class Move {
public:
    U8 id;

    eSquare from;
    eSquare to;

    ePieceType piece_from : 3;
    ePieceType piece_to : 3;

    eSpecialMoveFlags move_flag : 3;
    bool is_capture : 1;

    Move() : id(0),
             from(A1),
             to(A1),
             piece_from(NUM_PIECE_TYPES),
             piece_to(NUM_PIECE_TYPES),
             move_flag(MFLAG_NULLMOVE),
             is_capture(false) {}
    
    Move(eSquare from,
         eSquare to,
         ePieceType piece_from,
         ePieceType piece_to,
         eSpecialMoveFlags move_flag,
         bool is_capture,
         U8 id)
         :
         id(id),
         from(from),
         to(to),
         piece_from(piece_from),
         piece_to(piece_to),
         move_flag(move_flag),
         is_capture(is_capture) {}
    
    friend std::ostream& operator<<(std::ostream& os, const Move& m) {
        os << " BNRQK"[m.piece_from] << square_str(m.from) << (m.is_capture ? 'x' : '-') << square_str(m.to);
        return os;
    }
};