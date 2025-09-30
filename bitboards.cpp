#include "bitboards.hpp"

#include <cassert>

Bitboards::Bitboards() {
    for (eColor color = WHITE; color <= BLACK; color++)
        for (ePiece pt = PAWN; pt < NUM_PIECE_TYPES; pt++)
            piece_bbs[color][pt] = STARTING_PIECE_POSITIONS[color][pt];

    color_bbs[WHITE] = STARTING_COLOR_POSITIONS[WHITE];
    color_bbs[BLACK] = STARTING_COLOR_POSITIONS[BLACK];
    side_to_move = WHITE;
}


Bitboards::Bitboards(const Bitboards& other) {
    for (eColor color = WHITE; color <= BLACK; color++)
        for (ePiece pt = PAWN; pt < NUM_PIECE_TYPES; pt++)
            piece_bbs[color][pt] = other.piece_bbs[color][pt];

    color_bbs[WHITE] = other.color_bbs[WHITE];
    color_bbs[BLACK] = other.color_bbs[BLACK];
    side_to_move = other.side_to_move;
}


ePiece Bitboards::piece_at(eColor color, eSquare square) {
    for (ePiece pt = PAWN; pt < NUM_PIECE_TYPES; pt++)
        if (piece_bbs[color][pt].filled(square))
            return static_cast<ePiece>(pt);
    return NULL_PIECE;
}

eColor Bitboards::color_at(eSquare square) {
    if (color_bbs[WHITE].filled(square))
        return WHITE;
    else if (color_bbs[BLACK].filled(square))
        return BLACK;
    return NULL_COLOR;
}

void Bitboards::handle_move(const Move& move) {
    pickup_piece(move.from, side_to_move, move.piece_from);
    ePiece taken = put_piece(move.to, side_to_move, move.piece_from);
    side_to_move = !side_to_move;
}


// Put a piece down on the board. If there was another piece on that square previously,
// return the type of piece that was removed.
inline ePiece Bitboards::put_piece(eSquare square, eColor piece_color, ePiece piece_type) {
    color_bbs[piece_color].fill(square);
    piece_bbs[piece_color][piece_type].fill(square);

    return pickup_piece(square, !piece_color); // Remove and return the piece that was previously there.
}


// Remove a piece from the board
inline void Bitboards::pickup_piece(eSquare square, eColor piece_color, ePiece piece_type) {
    color_bbs[piece_color].zero(square);
    piece_bbs[piece_color][piece_type].zero(square);
}


// Remove the piece at the given square, return what kind of piece it was.
inline ePiece Bitboards::pickup_piece(eSquare square, eColor piece_color) {
    color_bbs[piece_color].zero(square);

    ePiece pt = piece_at(piece_color, square);
    if (pt != NULL_PIECE)
        piece_bbs[piece_color][pt].zero(square);
    return pt;
}