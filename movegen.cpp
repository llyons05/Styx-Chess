#include "movegen.hpp"

/* Generate a vector with all the legal moves in this position.
@param expected_best
    The index of the expected best move
@return
    A reference to a vector with all the legal moves.
*/
std::vector<Move>& Move_Generator::gen_moves() {
    gen_pawn_moves();
    return movelist;
}


inline void Move_Generator::write_moves(ePieceType piece, eSquare from, BB64 to) {
    while(to) {
        eSquare dest = to.pop();
        movelist.push_back(Move(
            from,
            dest,
            piece,
            piece,
            MFLAG_NORMAL,
            board.bbs.color_bbs[board.stm()].filled(dest),
            movelist.size()
        ));
    }
}

void Move_Generator::gen_pawn_moves() {
    if (bbs.side_to_move == WHITE) {
        BB64 pawn_bb = bbs[WHITE, PAWN];
        BB64 valid_moves = pawn_bb.shift<NORTH>();
        valid_moves &= ~bbs.all();
        valid_moves |= (valid_moves & ROWS[2]).shift<NORTH, 1>();
        valid_moves.print();
    }
}