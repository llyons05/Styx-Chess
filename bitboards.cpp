#include "bitboards.hpp"

#include <cassert>

Bitboards::Bitboards() {
    for (int color = 0; color < 2; color++)
        for (int pt = 0; pt < NUM_PIECE_TYPES; pt++)
            piece_bbs[color][pt] = STARTING_PIECE_POSITIONS[color][pt];

    color_bbs[WHITE] = STARTING_COLOR_POSITIONS[WHITE];
    color_bbs[BLACK] = STARTING_COLOR_POSITIONS[BLACK];
    side_to_move = WHITE;
}


Bitboards::Bitboards(const Bitboards& other) {
    for (int color = 0; color < 2; color++)
        for (int pt = 0; pt < NUM_PIECE_TYPES; pt++)
            piece_bbs[color][pt] = other.piece_bbs[color][pt];

    color_bbs[WHITE] = other.color_bbs[WHITE];
    color_bbs[BLACK] = other.color_bbs[BLACK];
    side_to_move = other.side_to_move;
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
    if (color_bbs[piece_color].filled(square)) // If piece not present in given color bb just return NULL_PIECE
        return NULL_PIECE;

    color_bbs[piece_color].zero(square);

    // Loop through piece bbs and remove the piece when found
    for (U8 pt = PAWN; pt < NUM_PIECE_TYPES; pt++) {
        if (piece_bbs[piece_color][pt].filled(square)) {
            piece_bbs[piece_color][pt].zero(square);
            return static_cast<ePiece>(pt);
        }
    }
    assert(false); // We should never get here
}