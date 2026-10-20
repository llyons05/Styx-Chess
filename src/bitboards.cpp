#include "bitboards.hpp"

#include <cassert>
#include <iostream>
#include <string>

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
    fifty_move_rule = 0;
    stm = WHITE;

    hash_key ^= zobrist.ep(ep_square);
    hash_key ^= zobrist.castling(castling_rights);
}


Bitboards::Bitboards(const Bitboards& other) {
    for (int i = 0; i < 64; i++)
        pieces[i] = Piece();
    
    for (eColor color = WHITE; color <= BLACK; color++) {
        color_bbs[color] = 0;
        for (ePieceType pt = PAWN; pt < NUM_PIECE_TYPES; pt++) {
            piece_bbs[color][pt] = 0;
            BB64 pieces = other.piece_bbs[color][pt];
            while (!pieces.empty()) {
                eSquare s = pieces.pop();
                put_piece(s, Piece(color, pt));
            }
        }
    }
    castling_rights = other.castling_rights;
    ep_square = other.ep_square;
    fifty_move_rule = other.fifty_move_rule;
    stm = other.stm;
    hash_key = other.hash_key;
}


Bitboards::Bitboards(std::string fen) {
    auto square_conv = [](int s){int row = 7-(s/8); int col = s%8; return eSquare(row*8 + col);};
    int curr_square = 0;
    size_t index = 0;
    for (; (index < fen.size()) && (fen[index] != ' '); index++) {
        char c = fen[index];
        if (c == '/')
            continue;
        else if (isdigit(c)) {
            assert((0 < c - '0') && (c - '0'<= 8));
            curr_square += c - '0';
        }
        else {
            put_piece(square_conv(curr_square), Piece(c));
            curr_square++;
        }
    }
    stm = fen[++index] == 'w' ? WHITE : BLACK;
    castling_rights = 0;
    for (index += 2; (index < fen.size()) && (fen[index] != ' '); index++) {
        char c = fen[index];
        if (c == 'k')
            castling_rights |= CASTLE_BK;
        else if (c == 'K')
            castling_rights |= CASTLE_WK;
        else if (c == 'q')
            castling_rights |= CASTLE_BQ;
        else if (c == 'Q')
            castling_rights |= CASTLE_WQ;
    }
    std::string ep_section = fen.substr(index + 1, 2);
    assert(ep_section.size() >= 1);
    if (ep_section[0] == '-')
        ep_square = NULL_SQUARE;
    else
        ep_square = str_to_square(ep_section);
    
    index += 4;
    if (index + 2 < fen.size())
        fifty_move_rule = std::stoi(fen.substr(index, 2));
    
    if (stm == BLACK)
        hash_key ^= zobrist.color();
    hash_key ^= zobrist.ep(ep_square);
    hash_key ^= zobrist.castling(castling_rights);
}


Piece Bitboards::piece_at(eSquare square) const {
    assert((square >= A1) && (square < NULL_SQUARE));
    return pieces[square];
}


eColor Bitboards::color_at(eSquare square) const {
    return pieces[square].color;
}


bool Bitboards::has_mating_material(eColor color) const {
    BB64 n_b_bb = piece_bbs[color][BISHOP] | piece_bbs[color][KNIGHT];
    return (color_bbs[color] != (piece_bbs[color][KING] | n_b_bb)) || (n_b_bb.bitcount() > 1);
}

Piece Bitboards::handle_move(const Move& move) {
    hash_key ^= zobrist.ep(ep_square);
    ep_square = NULL_SQUARE;
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

    else if (move.is_pawn_jump()) {
        update_ep_square(move);
    }

    else if (move.is_ep()) {
        if (stm == WHITE)
            pickup_piece(eSquare(move.to - 8));
        else
            pickup_piece(eSquare(move.to + 8));
    }

    if ((move.piece_from == PAWN) || move.is_capture())
        fifty_move_rule = 0;
    else
        fifty_move_rule++;

    update_castling_rights(move);
    change_turn();
    return taken_piece;
}


void Bitboards::undo_move(const Move& move, const BB_State_Info& state_info) {
    change_turn();
    fifty_move_rule = state_info.fifty_move_rule;

    hash_key ^= zobrist.castling(castling_rights);
    castling_rights = state_info.castling_rights;
    hash_key ^= zobrist.castling(castling_rights);

    hash_key ^= zobrist.ep(ep_square);
    ep_square = state_info.ep_square;
    hash_key ^= zobrist.ep(ep_square);

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
    else if (move.is_ep()) {
        if (stm == WHITE)
            put_piece(eSquare(move.to - 8), Piece(!stm, PAWN));
        else
            put_piece(eSquare(move.to + 8), Piece(!stm, PAWN));
    }

    pickup_piece(move.to);
    put_piece(move.from, Piece(stm, move.piece_from));
    if (move.is_capture() && !move.is_ep())
        put_piece(move.to, Piece(!stm, move.piece_captured));
}

// Put a piece down on the board. If there was another piece on that square previously,
// return the type of piece that was removed.
inline Piece Bitboards::put_piece(eSquare square, Piece piece) {
    assert(!piece.is_null());

    Piece taken_piece = pickup_piece(square);

    color_bbs[piece.color].fill(square);
    piece_bbs[piece.color][piece.pt].fill(square);
    pieces[square] = piece;

    hash_key ^= zobrist[piece.color, piece.pt, square];
    return taken_piece;
}


// Remove the piece at the given square, and return it.
inline Piece Bitboards::pickup_piece(eSquare square) {
    Piece piece = piece_at(square);
    if (!piece.is_null()) {
        color_bbs[piece.color].zero(square);
        piece_bbs[piece.color][piece.pt].zero(square);
        pieces[square] = Piece();
        hash_key ^= zobrist[piece.color, piece.pt, square];
    }
    return piece;
}


void Bitboards::update_castling_rights(const Move& m) {
    hash_key ^= zobrist.castling(castling_rights);
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
    hash_key ^= zobrist.castling(castling_rights);
}

void Bitboards::update_ep_square(const Move& m) {
    // In the future check if en passant is possible before setting it
    if (stm == WHITE)
        ep_square = eSquare(m.from + 8);
    else
        ep_square = eSquare(m.from - 8);
    
    hash_key ^= zobrist.ep(ep_square);
}

void Bitboards::change_turn() {
    stm = !stm;
    hash_key ^= zobrist.color();
}