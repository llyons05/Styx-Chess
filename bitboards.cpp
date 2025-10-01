#include "bitboards.hpp"

#include <cassert>
#include <iostream>

Bitboards::Bitboards() {
    for (eColor color = WHITE; color <= BLACK; color++)
        for (ePieceType pt = PAWN; pt < NUM_PIECE_TYPES; pt++)
            piece_bbs[color][pt] = STARTING_PIECE_POSITIONS[color][pt];

    color_bbs[WHITE] = STARTING_COLOR_POSITIONS[WHITE];
    color_bbs[BLACK] = STARTING_COLOR_POSITIONS[BLACK];
    side_to_move = WHITE;
}


Bitboards::Bitboards(const Bitboards& other) {
    for (eColor color = WHITE; color <= BLACK; color++)
        for (ePieceType pt = PAWN; pt < NUM_PIECE_TYPES; pt++)
            piece_bbs[color][pt] = other.piece_bbs[color][pt];

    color_bbs[WHITE] = other.color_bbs[WHITE];
    color_bbs[BLACK] = other.color_bbs[BLACK];
    side_to_move = other.side_to_move;
}


Piece Bitboards::piece_at(eSquare square) const {
    eColor color = color_at(square);
    ePieceType pt = color == NUM_COLORS? NUM_PIECE_TYPES : piece_at(color, square);
    return Piece(color, pt);
}


eColor Bitboards::color_at(eSquare square) const {
    if (color_bbs[WHITE].filled(square))
        return WHITE;
    else if (color_bbs[BLACK].filled(square))
        return BLACK;
    return NUM_COLORS;
}


ePieceType Bitboards::piece_at(eColor color, eSquare square) const {
    assert(color != NUM_COLORS);

    for (ePieceType pt = PAWN; pt < NUM_PIECE_TYPES; pt++)
        if (piece_bbs[color][pt].filled(square))
            return pt;
    return NUM_PIECE_TYPES;
}


void Bitboards::handle_move(const Move& move) {
    Piece moved_piece = Piece(side_to_move, move.piece_from);
    pickup_piece(move.from, moved_piece);
    Piece taken_piece = put_piece(move.to, moved_piece);
    side_to_move = !side_to_move;
}


// Remove a piece from the board
inline void Bitboards::pickup_piece(eSquare square, Piece piece) {
    assert(!piece.is_null());

    color_bbs[piece.color].zero(square);
    piece_bbs[piece.color][piece.pt].zero(square);
}


// Put a piece down on the board. If there was another piece on that square previously,
// return the type of piece that was removed.
inline Piece Bitboards::put_piece(eSquare square, Piece piece) {
    assert(!piece.is_null());

    color_bbs[piece.color].fill(square);
    piece_bbs[piece.color][piece.pt].fill(square);

    return pickup_piece(square, !piece.color); // Remove and return the piece that was previously there.
}


// Remove the piece at the given square, return what kind of piece it was.
inline Piece Bitboards::pickup_piece(eSquare square, eColor piece_color) {
    ePieceType pt = piece_at(piece_color, square);
    color_bbs[piece_color].zero(square);
    if (pt != NUM_PIECE_TYPES)
        piece_bbs[piece_color][pt].zero(square);
    return Piece(piece_color, pt);
}