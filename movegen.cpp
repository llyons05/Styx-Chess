#include "movegen.hpp"

template <eColor C>
std::vector<Move>& Move_Generator::gen_moves() {
    gen_pawn_moves<C>();
    gen_bishop_moves();
    gen_knight_moves();
    gen_rook_moves();
    gen_queen_moves();
    gen_king_moves();
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
            board.bbs.opp_bb().filled(dest),
            movelist.size()
        ));
    }
}

template<Direction D>
BB64 Move_Generator::valid_sliding_moves(BB64 pieces, Board& board) {
    BB64 valid_moves = 0;
    BB64 next = pieces.shift<D>() & ~board.bbs.stm_bb();
    while (!next.empty()) {
        valid_moves |= next;
        next = (next & ~board.bbs.opp_bb()).shift<D>() & ~board.bbs.stm_bb();
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

void Move_Generator::gen_bishop_moves() {
    BB64 bishops = bbs.stm_bb(BISHOP);
    while (!bishops.empty()) {
        eSquare s = bishops.pop();
        BB64 valid_moves = 0;
        valid_moves |= valid_sliding_moves<NORTHEAST>(s, board);
        valid_moves |= valid_sliding_moves<SOUTHEAST>(s, board);
        valid_moves |= valid_sliding_moves<SOUTHWEST>(s, board);
        valid_moves |= valid_sliding_moves<NORTHWEST>(s, board);
        write_moves(BISHOP, s, valid_moves);
    }
}

void Move_Generator::gen_knight_moves() {
    BB64 knights = bbs.stm_bb(KNIGHT);
    while (!knights.empty()) {
        eSquare s = knights.pop();
        BB64 valid_moves = knight_moves[s] & ~bbs.stm_bb();
        write_moves(KNIGHT, s, valid_moves);
    }
}

void Move_Generator::gen_rook_moves() {
    BB64 rooks = bbs.stm_bb(ROOK);
    while (!rooks.empty()) {
        eSquare s = rooks.pop();
        BB64 valid_moves = 0;
        valid_moves |= valid_sliding_moves<NORTH>(s, board);
        valid_moves |= valid_sliding_moves<EAST>(s, board);
        valid_moves |= valid_sliding_moves<SOUTH>(s, board);
        valid_moves |= valid_sliding_moves<WEST>(s, board);
        write_moves(ROOK, s, valid_moves);
    }
}

void Move_Generator::gen_queen_moves() {
    BB64 queens = bbs.stm_bb(QUEEN);
    while (!queens.empty()) {
        eSquare s = queens.pop();
        BB64 valid_moves = 0;
        valid_moves |= valid_sliding_moves<NORTH>(s, board);
        valid_moves |= valid_sliding_moves<EAST>(s, board);
        valid_moves |= valid_sliding_moves<SOUTH>(s, board);
        valid_moves |= valid_sliding_moves<WEST>(s, board);
        valid_moves |= valid_sliding_moves<NORTHEAST>(s, board);
        valid_moves |= valid_sliding_moves<SOUTHEAST>(s, board);
        valid_moves |= valid_sliding_moves<SOUTHWEST>(s, board);
        valid_moves |= valid_sliding_moves<NORTHWEST>(s, board);
        write_moves(QUEEN, s, valid_moves);
    }
}

void Move_Generator::gen_king_moves() {
    BB64 king = bbs.stm_bb(KING);
    eSquare s = king.pop();
    BB64 valid_moves = king_moves[s] & ~bbs.stm_bb();
    write_moves(KING, s, valid_moves);
    assert(king.empty());
}


bool Move_Generator::is_attacked(Board& board, eSquare square) {
    BB64 bb(square);
    BB64 pawn_threats = (board.bbs.stm == WHITE) ? pawn_attacks<WHITE>(board) : pawn_attacks<BLACK>(board);
    if ((bb & pawn_threats) != 0)
        return true;

    BB64 knights = board.bbs.stm_bb(KNIGHT);
    BB64 knight_threats = 0;
    while (!knights.empty())
        knight_threats |= knight_moves[knights.pop()] & ~board.bbs.stm_bb();
    if ((bb & knight_threats) != 0)
        return true;

    BB64 diags = board.bbs.stm_bb(BISHOP) | board.bbs.stm_bb(QUEEN);
    BB64 diag_threats = valid_sliding_moves<NORTHEAST>(diags, board);
    diag_threats |= valid_sliding_moves<SOUTHEAST>(diags, board);
    diag_threats |= valid_sliding_moves<SOUTHWEST>(diags, board);
    diag_threats |= valid_sliding_moves<NORTHWEST>(diags, board);
    if ((bb & diag_threats) != 0)
        return true;
    
    BB64 straights = board.bbs.stm_bb(ROOK) | board.bbs.stm_bb(QUEEN);
    BB64 straight_threats = valid_sliding_moves<NORTH>(straights, board);
    straight_threats |= valid_sliding_moves<EAST>(straights, board);
    straight_threats |= valid_sliding_moves<SOUTH>(straights, board);
    straight_threats |= valid_sliding_moves<WEST>(straights, board);
    if ((bb & straight_threats) != 0)
        return true;
    
    BB64 king = board.bbs.stm_bb(KING);
    BB64 king_threats = king_moves[king.pop()] & ~board.bbs.stm_bb();
    return (bb & king_threats) != 0;
}


template <eColor C>
BB64 Move_Generator::pawn_attacks(Board& board) {
    constexpr Direction take1 = (C == WHITE) ? NORTHEAST : SOUTHWEST;
    constexpr Direction take2 = (C == WHITE) ? NORTHWEST : SOUTHEAST;
    BB64 pawns = board.bbs[C, PAWN];
    BB64 result = pawns.shift<take1>();
    return result | pawns.shift<take2>();
}

template std::vector<Move>& Move_Generator::gen_moves<WHITE>();
template std::vector<Move>& Move_Generator::gen_moves<BLACK>();