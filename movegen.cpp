#include "movegen.hpp"

/* Generate a vector with all the legal moves in this position.
@param expected_best
    The index of the expected best move
@return
    A reference to a vector with all the legal moves.
*/
template <eColor C>
std::vector<Move>& Move_Generator::gen_moves() {
    gen_pawn_moves<C>();
    return movelist;
}

void Move_Generator::write_moves(ePieceType piece, eSquare from, BB64 to) {
    while(to) {
        eSquare dest = to.pop();
        movelist.push_back(Move(
            from,
            dest,
            piece,
            piece,
            MFLAG_NORMAL,
            board.bbs.color_bbs[!board.stm()].filled(dest),
            movelist.size()
        ));
    }
}

template <eColor C>
void Move_Generator::gen_pawn_moves() {
    constexpr Direction forward = (C == WHITE) ? NORTH : SOUTH;
    constexpr Direction take1 = (C == WHITE) ? NORTHEAST : SOUTHWEST;
    constexpr Direction take2 = (C == WHITE) ? NORTHWEST : SOUTHEAST;
    BB64 pawns = bbs[C, PAWN];
    while (!pawns.empty()) {
        eSquare s = pawns.pop();
        BB64 p(s);
        BB64 valid_moves = p.shift<forward>();
        valid_moves &= ~bbs.all();
        valid_moves |= (valid_moves & ROWS[relative_row(C, 2)]).shift<forward>();
        valid_moves |= (p.shift<take1>() | p.shift<take2>()) & bbs[!C];
        write_moves(PAWN, s, valid_moves);
    }
}

template std::vector<Move>& Move_Generator::gen_moves<WHITE>();
template std::vector<Move>& Move_Generator::gen_moves<BLACK>();