#pragma once

#include "utils.hpp"

class Move {
public:
    Score score;

    eSquare from : 7;
    eSquare to : 7;

    ePieceType piece_from : 3;
    ePieceType piece_to : 3;
    ePieceType piece_captured : 3;

    eSpecialMoveFlags move_flag : 3;

    Move() : score(0),
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
         eSpecialMoveFlags move_flag)
         :
         score(0),
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
        return move_flag == MFLAG_NULLMOVE;
    }

    inline bool is_castle() const {
        return move_flag == MFLAG_CASTLE;
    }

    inline bool is_pawn_jump() const {
        return (piece_from == PAWN) && (abs(int(to) - int(from)) == 16);
    }

    inline bool is_ep() const {
        return move_flag == MFLAG_EP;
    }

    inline bool is_promo() const {
        return move_flag == MFLAG_PROMO;
    }

    inline bool is_valid() const {
        return from != to;
    }

    std::string long_alg() const {
        std::string result = square_str(from) + square_str(to);
        if (is_promo())
            result += " bnrqk"[piece_to];
        return result;
    }
    
    friend std::ostream& operator<<(std::ostream& os, const Move& m) {
        if (m.is_castle() && (m.to == G1 || m.to == G8))
            os << "O-O";
        else if (m.is_castle() && (m.to == C1 || m.to == C8))
            os << "O-O-O";
        else
            os << " BNRQK"[m.piece_from] << square_str(m.from) << (m.is_capture() ? 'x' : '-') << square_str(m.to);
        if (m.is_promo())
            os << "=" << " BNRQK"[m.piece_to];
        return os;
    }
};