#include "bitboards.hpp"

#include <cassert>

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


ePiece Bitboards::piece_at(eSquare square) const {
    eColor color = color_at(square);
    return color & piece_at(color, square);
}


ePieceType Bitboards::piece_at(eColor color, eSquare square) const {
    for (ePieceType pt = PAWN; pt < NUM_PIECE_TYPES; pt++)
        if (piece_bbs[color][pt].filled(square))
            return static_cast<ePieceType>(pt);
    return NULL_PT;
}

eColor Bitboards::color_at(eSquare square) const {
    if (color_bbs[WHITE].filled(square))
        return WHITE;
    else if (color_bbs[BLACK].filled(square))
        return BLACK;
    return NULL_COLOR;
}

void Bitboards::handle_move(const Move& move) {
    pickup_piece(move.from, side_to_move & move.piece_from);
    ePiece taken = put_piece(move.to, side_to_move & move.piece_to);
    side_to_move = !side_to_move;
}


// Put a piece down on the board. If there was another piece on that square previously,
// return the type of piece that was removed.
inline ePiece Bitboards::put_piece(eSquare square, ePiece piece) {
    color_bbs[color_of(piece)].fill(square);
    piece_bbs[color_of(piece)][type_of(piece)].fill(square);

    return pickup_piece(square, !color_of(piece)); // Remove and return the piece that was previously there.
}


// Remove a piece from the board
inline void Bitboards::pickup_piece(eSquare square, ePiece piece) {
    color_bbs[color_of(piece)].zero(square);
    piece_bbs[color_of(piece)][type_of(piece)].zero(square);
}


// Remove the piece at the given square, return what kind of piece it was.
inline ePiece Bitboards::pickup_piece(eSquare square, eColor piece_color) {
    color_bbs[piece_color].zero(square);

    ePieceType pt = piece_at(piece_color, square);
    if (pt != NULL_PT)
        piece_bbs[piece_color][pt].zero(square);
    return piece_color & pt;
}