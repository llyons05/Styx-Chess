#pragma once

#include "utils.hpp"

class Move {
public:
    eSquare from : 7;
    eSquare to : 7;

    ePieceType piece_from : 3;
    ePieceType piece_to : 3;
    ePieceType piece_captured : 3;

    eSpecialMoveFlags move_flag : 3;

    U8 prev_castle_flags : 4;
    eSquare prev_ep_square : 7;

    Move() : from(A1),
             to(A1),
             piece_from(NUM_PIECE_TYPES),
             piece_to(NUM_PIECE_TYPES),
             piece_captured(NUM_PIECE_TYPES),
             move_flag(MFLAG_NORMAL),
             prev_castle_flags(0), 
             prev_ep_square(NULL_SQUARE) {}
    
    Move(eSquare from,
         eSquare to,
         ePieceType piece_from,
         ePieceType piece_to,
         ePieceType piece_captured,
         eSpecialMoveFlags move_flag,
         U8 prev_castle_flags,
         eSquare prev_ep_square)
         :
         from(from),
         to(to),
         piece_from(piece_from),
         piece_to(piece_to),
         piece_captured(piece_captured),
         move_flag(move_flag),
         prev_castle_flags(prev_castle_flags),
         prev_ep_square(prev_ep_square) {}

    inline bool is_capture() const {
        return piece_captured != NUM_PIECE_TYPES;
    }

    inline bool is_nullmove() const {
        return move_flag & MFLAG_NULLMOVE;
    }

    inline bool is_castle() const {
        return move_flag & MFLAG_CASTLE;
    }
    
    friend std::ostream& operator<<(std::ostream& os, const Move& m) {
        if (m.is_castle() && (m.to == G1 || m.to == G8))
            os << "O-O";
        else if (m.is_castle() && (m.to == C1 || m.to == C8) )
            os << "O-O-O";
        else
            os << " BNRQK"[m.piece_from] << square_str(m.from) << (m.is_capture() ? 'x' : '-') << square_str(m.to);
        return os;
    }
};