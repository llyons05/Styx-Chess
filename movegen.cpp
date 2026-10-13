#include "movegen.hpp"

template <eColor C>
std::vector<Move>& Move_Generator::gen_moves() {
    gen_pawn_moves<C>();
    gen_bishop_moves<C>();
    gen_knight_moves<C>();
    gen_rook_moves<C>();
    gen_queen_moves<C>();
    gen_king_moves<C>();
    return movelist;
}

void Move_Generator::write_moves(ePieceType piece, eSquare from, BB64 to) {
    destinations |= to;
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

template<eColor C, Direction D>
BB64 Move_Generator::valid_sliding_moves(eSquare s) {
    BB64 valid_moves = 0;
    BB64 next = BB64(s).shift<D>();
    while (!next.empty() && ((next & bbs[C]) == 0)) {
        valid_moves |= next;
        if ((next & bbs[!C]) != 0)
            break;
        next = next.shift<D>();
    }
    return valid_moves;
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
        BB64 valid_moves = p.shift<forward>() & ~bbs.all();
        valid_moves |= (valid_moves & ROWS[relative_row(C, 2)]).shift<forward>() & ~bbs.all();
        valid_moves |= (p.shift<take1>() | p.shift<take2>()) & bbs[!C];
        write_moves(PAWN, s, valid_moves);
    }
}

template <eColor C>
void Move_Generator::gen_bishop_moves() {
    BB64 bishops = bbs[C, BISHOP];
    while (!bishops.empty()) {
        eSquare s = bishops.pop();
        BB64 valid_moves = 0;
        valid_moves |= valid_sliding_moves<C, NORTHEAST>(s);
        valid_moves |= valid_sliding_moves<C, SOUTHEAST>(s);
        valid_moves |= valid_sliding_moves<C, SOUTHWEST>(s);
        valid_moves |= valid_sliding_moves<C, NORTHWEST>(s);
        write_moves(BISHOP, s, valid_moves);
    }
}

template <eColor C>
void Move_Generator::gen_knight_moves() {
    BB64 knights = bbs[C, KNIGHT];
    while (!knights.empty()) {
        eSquare s = knights.pop();
        BB64 valid_moves = knight_moves[s] & ~bbs[C];
        write_moves(KNIGHT, s, valid_moves);
    }
}

template <eColor C>
void Move_Generator::gen_rook_moves() {
    BB64 rooks = bbs[C, ROOK];
    while (!rooks.empty()) {
        eSquare s = rooks.pop();
        BB64 valid_moves = 0;
        valid_moves |= valid_sliding_moves<C, NORTH>(s);
        valid_moves |= valid_sliding_moves<C, EAST>(s);
        valid_moves |= valid_sliding_moves<C, SOUTH>(s);
        valid_moves |= valid_sliding_moves<C, WEST>(s);
        write_moves(ROOK, s, valid_moves);
    }
}

template <eColor C>
void Move_Generator::gen_queen_moves() {
    BB64 queens = bbs[C, QUEEN];
    while (!queens.empty()) {
        eSquare s = queens.pop();
        BB64 valid_moves = 0;
        valid_moves |= valid_sliding_moves<C, NORTH>(s);
        valid_moves |= valid_sliding_moves<C, EAST>(s);
        valid_moves |= valid_sliding_moves<C, SOUTH>(s);
        valid_moves |= valid_sliding_moves<C, WEST>(s);
        valid_moves |= valid_sliding_moves<C, NORTHEAST>(s);
        valid_moves |= valid_sliding_moves<C, SOUTHEAST>(s);
        valid_moves |= valid_sliding_moves<C, SOUTHWEST>(s);
        valid_moves |= valid_sliding_moves<C, NORTHWEST>(s);
        write_moves(QUEEN, s, valid_moves);
    }
}

template <eColor C>
void Move_Generator::gen_king_moves() {
    BB64 king = bbs[C, KING];
    eSquare s = king.pop();
    BB64 valid_moves = king_moves[s] & ~bbs[C];
    write_moves(KING, s, valid_moves);
    assert(king.empty());
}

template std::vector<Move>& Move_Generator::gen_moves<WHITE>();
template std::vector<Move>& Move_Generator::gen_moves<BLACK>();