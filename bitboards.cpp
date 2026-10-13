#include "bitboards.hpp"

#include <cassert>
#include <iostream>

void BB64::print() const {
    for (int i = 56; i >= 0; i -= 8) {
        for (int j = 0; j < 8; j++) {
            if (filled(eSquare(i + j))) {
                std::cout << "1 "; 
            }
            else {
                std::cout << "O ";
            }
        }
        std::cout << "\n";
    }
}

Bitboards::Bitboards() {
    for (eColor color = WHITE; color <= BLACK; color++)
        for (ePieceType pt = PAWN; pt < NUM_PIECE_TYPES; pt++) {
            BB64 pieces = STARTING_PIECE_POSITIONS[color][pt];
            while (!pieces.empty()) {
                eSquare s = pieces.pop();
                put_piece(s, Piece(color, pt));
            }
        }
    castling_rights = CASTLE_WK | CASTLE_WQ | CASTLE_BK | CASTLE_BQ;
    ep_square = NULL_SQUARE;
    stm = WHITE;
}


Bitboards::Bitboards(const Bitboards& other) {
    for (eColor color = WHITE; color <= BLACK; color++)
        for (ePieceType pt = PAWN; pt < NUM_PIECE_TYPES; pt++) {
            BB64 pieces = other.piece_bbs[color][pt];
            while (!pieces.empty()) {
                eSquare s = pieces.pop();
                put_piece(s, Piece(color, pt));
            }
        }
    castling_rights = other.castling_rights;
    ep_square = other.ep_square;
    stm = other.stm;
}


Piece Bitboards::piece_at(eSquare square) const {
    assert((square >= A1) && (square < NULL_SQUARE));
    return pieces[square];
}


eColor Bitboards::color_at(eSquare square) const {
    return pieces[square].color;
}


Piece Bitboards::handle_move(const Move& move) {
    pickup_piece(move.from);
    Piece taken_piece = put_piece(move.to, Piece(stm, move.piece_to));
    if (move.is_castle()) {
        if (move.to == G1)      // White kingside
            put_piece(F1, pickup_piece(H1));
        else if (move.to == C1) // White queenside
            put_piece(D1, pickup_piece(A1));
        else if (move.to == G8) // Black kingside
            put_piece(F8, pickup_piece(H8));
        else if (move.to == C8) // Black queenside
            put_piece(D8, pickup_piece(A8));
    }

    update_castling_rights(move);
    stm = !stm;
    return taken_piece;
}

void Bitboards::undo_move(const Move& move) {
    stm = !stm;
    castling_rights = move.prev_castle_flags;
    if (move.is_castle()) {
        if (move.to == G1)     // White kingside
            put_piece(H1, pickup_piece(F1));
        else if (move.to == C1) // White queenside
            put_piece(A1, pickup_piece(D1));
        else if (move.to == G8) // Black kingside
            put_piece(H8, pickup_piece(F8));
        else if (move.to == C8) // Black queenside
            put_piece(A8, pickup_piece(D8));
    }

    pickup_piece(move.to);
    put_piece(move.from, Piece(stm, move.piece_from));
    if (move.piece_captured != NUM_PIECE_TYPES)
        put_piece(move.to, Piece(!stm, move.piece_captured));
}

// Put a piece down on the board. If there was another piece on that square previously,
// return the type of piece that was removed.
inline Piece Bitboards::put_piece(eSquare square, Piece piece) {
    assert(!piece.is_null());

    color_bbs[piece.color].fill(square);
    piece_bbs[piece.color][piece.pt].fill(square);

    Piece taken_piece = pickup_piece(square);
    pieces[square] = piece;
    return taken_piece;
}


// Remove the piece at the given square, and return it.
inline Piece Bitboards::pickup_piece(eSquare square) {
    Piece piece = piece_at(square);
    if (!piece.is_null()) {
        color_bbs[piece.color].zero(square);
        piece_bbs[piece.color][piece.pt].zero(square);
        pieces[square] = Piece();
    }
    return piece;
}


void Bitboards::update_castling_rights(const Move& m) {
    switch (m.from) {
    case H1:
        castling_rights &= ~CASTLE_WK;
        break;
    case E1:
        castling_rights &= ~(CASTLE_WK|CASTLE_WQ);
        break;
    case A1:
        castling_rights &= ~CASTLE_WQ;
        break;
    case H8:
        castling_rights &= ~CASTLE_BK;
        break;
    case E8:
        castling_rights &= ~(CASTLE_BK|CASTLE_BQ);
        break;
    case A8:
        castling_rights &= ~CASTLE_BQ;
        break;
    default:
        break;
    }
    switch (m.to) {
    case H1:
        castling_rights &= ~CASTLE_WK;
        break;
    case E1:
        castling_rights &= ~(CASTLE_WK|CASTLE_WQ);
        break;
    case A1:
        castling_rights &= ~CASTLE_WQ;
        break;
    case H8:
        castling_rights &= ~CASTLE_BK;
        break;
    case E8:
        castling_rights &= ~(CASTLE_BK|CASTLE_BQ);
        break;
    case A8:
        castling_rights &= ~CASTLE_BQ;
        break;
    default:
        break;
    }
}