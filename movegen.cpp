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

void Move_Generator::write_moves(ePieceType piece_from, ePieceType piece_to, eSquare from, BB64 to, eSpecialMoveFlags flag) {
    destinations |= to;
    while(to) {
        eSquare dest = to.pop();
        movelist.push_back(Move(
            from,
            dest,
            piece_from,
            piece_to,
            (flag == MFLAG_EP) ? PAWN : (bbs.opp_bb().filled(dest) ? bbs.piece_at(dest).pt : NUM_PIECE_TYPES),
            flag,
            movelist.size()
        ));
    }
}

template<Direction D>
BB64 Move_Generator::valid_sliding_moves(BB64 pieces, const Board& board, eColor attackers) {
    BB64 valid_moves = 0;
    BB64 next = pieces.shift<D>() & ~board.bbs[attackers];
    while (!next.empty()) {
        valid_moves |= next;
        next = (next & ~board.bbs[!attackers]).shift<D>() & ~board.bbs[attackers];
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
        write_moves(PAWN, PAWN, s, valid_moves, MFLAG_NORMAL);
    }
}

void Move_Generator::gen_bishop_moves() {
    BB64 bishops = bbs.stm_bb(BISHOP);
    while (!bishops.empty()) {
        eSquare s = bishops.pop();
        BB64 valid_moves = 0;
        valid_moves |= valid_sliding_moves<NORTHEAST>(s, board, board.stm());
        valid_moves |= valid_sliding_moves<SOUTHEAST>(s, board, board.stm());
        valid_moves |= valid_sliding_moves<SOUTHWEST>(s, board, board.stm());
        valid_moves |= valid_sliding_moves<NORTHWEST>(s, board, board.stm());
        write_moves(BISHOP, BISHOP, s, valid_moves, MFLAG_NORMAL);
    }
}

void Move_Generator::gen_knight_moves() {
    BB64 knights = bbs.stm_bb(KNIGHT);
    while (!knights.empty()) {
        eSquare s = knights.pop();
        BB64 valid_moves = knight_moves[s] & ~bbs.stm_bb();
        write_moves(KNIGHT, KNIGHT, s, valid_moves, MFLAG_NORMAL);
    }
}

void Move_Generator::gen_rook_moves() {
    BB64 rooks = bbs.stm_bb(ROOK);
    while (!rooks.empty()) {
        eSquare s = rooks.pop();
        BB64 valid_moves = 0;
        valid_moves |= valid_sliding_moves<NORTH>(s, board, board.stm());
        valid_moves |= valid_sliding_moves<EAST>(s, board, board.stm());
        valid_moves |= valid_sliding_moves<SOUTH>(s, board, board.stm());
        valid_moves |= valid_sliding_moves<WEST>(s, board, board.stm());
        write_moves(ROOK, ROOK, s, valid_moves, MFLAG_NORMAL);
    }
}

void Move_Generator::gen_queen_moves() {
    BB64 queens = bbs.stm_bb(QUEEN);
    while (!queens.empty()) {
        eSquare s = queens.pop();
        BB64 valid_moves = 0;
        valid_moves |= valid_sliding_moves<NORTH>(s, board, board.stm());
        valid_moves |= valid_sliding_moves<EAST>(s, board, board.stm());
        valid_moves |= valid_sliding_moves<SOUTH>(s, board, board.stm());
        valid_moves |= valid_sliding_moves<WEST>(s, board, board.stm());
        valid_moves |= valid_sliding_moves<NORTHEAST>(s, board, board.stm());
        valid_moves |= valid_sliding_moves<SOUTHEAST>(s, board, board.stm());
        valid_moves |= valid_sliding_moves<SOUTHWEST>(s, board, board.stm());
        valid_moves |= valid_sliding_moves<NORTHWEST>(s, board, board.stm());
        write_moves(QUEEN, QUEEN, s, valid_moves, MFLAG_NORMAL);
    }
}

void Move_Generator::gen_king_moves() {
    BB64 king = bbs.stm_bb(KING);
    eSquare s = king.lsb();
    BB64 valid_moves = king_moves[s] & ~bbs.stm_bb();
    write_moves(KING, KING, s, valid_moves, MFLAG_NORMAL);
}


bool Move_Generator::is_attacked(const Board& board, eColor attackers, BB64 bb) {
    BB64 pawn_threats = (attackers == WHITE) ? pawn_attacks<WHITE>(board) : pawn_attacks<BLACK>(board);
    if ((bb & pawn_threats) != 0)
        return true;

    BB64 knights = board.bbs[attackers, KNIGHT];
    BB64 knight_threats = 0;
    while (!knights.empty())
        knight_threats |= knight_moves[knights.pop()] & ~board.bbs[attackers];
    if ((bb & knight_threats) != 0)
        return true;

    BB64 diags = board.bbs[attackers, BISHOP] | board.bbs[attackers, QUEEN];
    BB64 diag_threats = valid_sliding_moves<NORTHEAST>(diags, board, attackers);
    diag_threats |= valid_sliding_moves<SOUTHEAST>(diags, board, attackers);
    diag_threats |= valid_sliding_moves<SOUTHWEST>(diags, board, attackers);
    diag_threats |= valid_sliding_moves<NORTHWEST>(diags, board, attackers);
    if ((bb & diag_threats) != 0)
        return true;
    
    BB64 straights = board.bbs[attackers, ROOK] | board.bbs[attackers, QUEEN];
    BB64 straight_threats = valid_sliding_moves<NORTH>(straights, board, attackers);
    straight_threats |= valid_sliding_moves<EAST>(straights, board, attackers);
    straight_threats |= valid_sliding_moves<SOUTH>(straights, board, attackers);
    straight_threats |= valid_sliding_moves<WEST>(straights, board, attackers);
    if ((bb & straight_threats) != 0)
        return true;
    
    BB64 king = board.bbs[attackers, KING];
    BB64 king_threats = king_moves[king.pop()] & ~board.bbs[attackers];
    if (attackers != board.stm())
        (pawn_threats | knight_threats | diag_threats | straight_threats | king_threats).print();
    return (bb & king_threats) != 0;
}


template <eColor C>
BB64 Move_Generator::pawn_attacks(const Board& board) {
    constexpr Direction take1 = (C == WHITE) ? NORTHEAST : SOUTHWEST;
    constexpr Direction take2 = (C == WHITE) ? NORTHWEST : SOUTHEAST;
    BB64 pawns = board.bbs[C, PAWN];
    BB64 result = pawns.shift<take1>();
    return result | pawns.shift<take2>();
}

template std::vector<Move>& Move_Generator::gen_moves<WHITE>();
template std::vector<Move>& Move_Generator::gen_moves<BLACK>();