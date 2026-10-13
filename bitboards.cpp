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
    side_to_move = WHITE;
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
    side_to_move = other.side_to_move;
}


Piece Bitboards::piece_at(eSquare square) const {
    assert((square >= A1) && (square < NULL_SQUARE));
    return pieces[square];
}


eColor Bitboards::color_at(eSquare square) const {
    return pieces[square].color;
}


Piece Bitboards::handle_move(const Move& move) {
    Piece moved_piece = Piece(side_to_move, move.piece_from);
    pickup_piece(move.from);
    Piece taken_piece = put_piece(move.to, moved_piece);
    side_to_move = !side_to_move;
    return taken_piece;
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