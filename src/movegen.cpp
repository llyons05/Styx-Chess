#include "movegen.hpp"

PRNG magic_rng(1070372);
std::array<Magic_Table<BISHOP>, 64> bishop_magics;
std::array<Magic_Table<ROOK>, 64> rook_magics;

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
BB64 Move_Generator::sliding_threats(BB64 pieces, const Board& board, eColor attackers) {
    BB64 valid_moves = 0;
    BB64 all = board.bbs.all();
    BB64 next = pieces.shift<D>();
    while (!next.empty()) {
        valid_moves |= next;
        next = (next & ~all).shift<D>();
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
        
        BB64 takes = (p.shift<take1>() | p.shift<take2>());
        valid_moves |= takes & bbs[!C];
        
        if ((p & ROWS[relative_row(C, 6)]).empty()) { // If we're not on the second-to-last row, do normal moves
            write_moves(PAWN, PAWN, s, valid_moves, MFLAG_NORMAL);
        }
        else { // If we ARE on the second-to-last row, all of our moves will result in a promotion.
            write_moves(PAWN, BISHOP, s, valid_moves, MFLAG_PROMO);
            write_moves(PAWN, KNIGHT, s, valid_moves, MFLAG_PROMO);
            write_moves(PAWN, ROOK, s, valid_moves, MFLAG_PROMO);
            write_moves(PAWN, QUEEN, s, valid_moves, MFLAG_PROMO);
        }

        // En passant stuff
        if ((bbs.ep_square != NULL_SQUARE) && !(p & ROWS[relative_row(C, 4)]).empty())
            write_moves(PAWN, PAWN, s, takes & BB64(bbs.ep_square), MFLAG_EP);
    }
}

void Move_Generator::gen_bishop_moves() {
    BB64 bishops = bbs.stm_bb(BISHOP);
    while (!bishops.empty()) {
        eSquare s = bishops.pop();
        BB64 valid_moves = bishop_magics[s].moves(board.bbs.all());
        write_moves(BISHOP, BISHOP, s, valid_moves & ~board.bbs.stm_bb(), MFLAG_NORMAL);
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
        BB64 valid_moves = rook_magics[s].moves(board.bbs.all());
        write_moves(ROOK, ROOK, s, valid_moves & ~board.bbs.stm_bb(), MFLAG_NORMAL);
    }
}

void Move_Generator::gen_queen_moves() {
    BB64 queens = bbs.stm_bb(QUEEN);
    while (!queens.empty()) {
        eSquare s = queens.pop();
        BB64 valid_moves = bishop_magics[s].moves(board.bbs.all()) | rook_magics[s].moves(board.bbs.all());
        write_moves(QUEEN, QUEEN, s, valid_moves & ~board.bbs.stm_bb(), MFLAG_NORMAL);
    }
}

void Move_Generator::gen_king_moves() {
    BB64 king = bbs.stm_bb(KING);
    eSquare s = king.lsb();
    BB64 valid_moves = king_moves[s] & ~bbs.stm_bb();
    write_moves(KING, KING, s, valid_moves, MFLAG_NORMAL);
    if (bbs.stm == WHITE) {
        if ((bbs.castling_rights & CASTLE_WK)
         && bbs.empty(BB64(F1) | BB64(G1))
         && !is_attacked(board, BLACK, BB64(E1) | BB64(F1) | BB64(G1)))
            write_moves(KING, KING, s, G1, MFLAG_CASTLE);
        if ((bbs.castling_rights & CASTLE_WQ)
         && bbs.empty(BB64(D1) | BB64(C1) | BB64(B1))
         && !is_attacked(board, BLACK, BB64(E1) | BB64(D1) | BB64(C1)))
            write_moves(KING, KING, s, C1, MFLAG_CASTLE);
    }
    else {
        if ((bbs.castling_rights & CASTLE_BK)
         && bbs.empty(BB64(F8) | BB64(G8))
         && !is_attacked(board, WHITE, BB64(E8) | BB64(F8) | BB64(G8)))
            write_moves(KING, KING, s, G8, MFLAG_CASTLE);
        if ((bbs.castling_rights & CASTLE_BQ)
         && bbs.empty(BB64(D8) | BB64(C8) | BB64(B8))
         && !is_attacked(board, WHITE, BB64(E8) | BB64(D8) | BB64(C8)))
            write_moves(KING, KING, s, C8, MFLAG_CASTLE);
    }
}

BB64 Move_Generator::attacks(const Board& board, eColor attackers) {
    BB64 result = (attackers == WHITE) ? pawn_attacks<WHITE>(board) : pawn_attacks<BLACK>(board);

    BB64 knights = board.bbs[attackers, KNIGHT];
    while (!knights.empty())
        result |= knight_moves[knights.pop()];

    BB64 diags = board.bbs[attackers, BISHOP] | board.bbs[attackers, QUEEN];
    while (!diags.empty())
        result |= bishop_magics[diags.pop()].moves(board.bbs.all());
    
    BB64 straights = board.bbs[attackers, ROOK] | board.bbs[attackers, QUEEN];
    while (!straights.empty())
        result |= rook_magics[straights.pop()].moves(board.bbs.all());
    
    BB64 king = board.bbs[attackers, KING];
    result |= king_moves[king.lsb()];
    return result;
}

bool Move_Generator::is_attacked(const Board& board, eColor attackers, BB64 bb) {
    BB64 pawn_threats = (attackers == WHITE) ? pawn_attacks<WHITE>(board) : pawn_attacks<BLACK>(board);
    if ((bb & pawn_threats) != 0)
        return true;

    BB64 knights = board.bbs[attackers, KNIGHT];
    while (!knights.empty())
        if ((bb & knight_moves[knights.pop()]) != 0)
            return true;

    BB64 diags = board.bbs[attackers, BISHOP] | board.bbs[attackers, QUEEN];
    while (!diags.empty())
        if ((bb & bishop_magics[diags.pop()].moves(board.bbs.all())) != 0)
            return true;
    
    BB64 straights = board.bbs[attackers, ROOK] | board.bbs[attackers, QUEEN];
    while (!straights.empty())
        if ((bb & rook_magics[straights.pop()].moves(board.bbs.all())) != 0)
            return true;
    
    BB64 king = board.bbs[attackers, KING];
    BB64 king_threats = king_moves[king.pop()] & ~board.bbs[attackers];
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

template <ePieceType pt>
BB64 Magic_Table<pt>::rel_occupancy_bits(eSquare s) {
    BB64 result = 0;
    if (pt == BISHOP) {
        result |= BB64(s).smear<NORTHEAST>();
        result |= BB64(s).smear<SOUTHEAST>();
        result |= BB64(s).smear<SOUTHWEST>();
        result |= BB64(s).smear<NORTHWEST>();
        result &= ~(ROWS[0] | ROWS[7] | COLS[FILE_A] | COLS[FILE_H]);
    }
    else {
        int row = square_row(s);
        int col = square_col(s);
        result |= ROWS[row] & ~(COLS[FILE_A] | COLS[FILE_H]);
        result |= COLS[col] & ~(ROWS[0] | ROWS[7]);
    }
    return result & ~BB64(s);
}

template <ePieceType pt>
BB64 Magic_Table<pt>::attack_bb(eSquare s, BB64 occupied) {
    constexpr Direction Dir1 = pt == BISHOP ? NORTHEAST : NORTH;
    constexpr Direction Dir2 = pt == BISHOP ? SOUTHEAST : EAST;
    constexpr Direction Dir3 = pt == BISHOP ? SOUTHWEST : SOUTH;
    constexpr Direction Dir4 = pt == BISHOP ? NORTHWEST : WEST;
    
    BB64 valid_moves = 0;
    BB64 next = BB64(s).shift<Dir1>();
    while (!next.empty()) {
        valid_moves |= next;
        next = (next & ~occupied).shift<Dir1>();
    }
    next = BB64(s).shift<Dir2>();
    while (!next.empty()) {
        valid_moves |= next;
        next = (next & ~occupied).shift<Dir2>();
    }
    next = BB64(s).shift<Dir3>();
    while (!next.empty()) {
        valid_moves |= next;
        next = (next & ~occupied).shift<Dir3>();
    }
    next = BB64(s).shift<Dir4>();
    while (!next.empty()) {
        valid_moves |= next;
        next = (next & ~occupied).shift<Dir4>();
    }
    return valid_moves;
}

template <ePieceType pt>
BB64 Magic_Table<pt>::blocker_index_mask(BB64 occupancy_bits, int blocker_index) {
    BB64 result = 0;
    int n = occupancy_bits.bitcount();
    for (int i = 0; i < n; i++) {
        eSquare s = occupancy_bits.pop();
        if (blocker_index & (1 << i)) result |= s;
    }
    return result;
}

template <ePieceType pt>
size_t Magic_Table<pt>::blockers_to_index(BB64 blockers) {
    return (U64(blockers) * magic) >> shift;
}

template <ePieceType pt>
void Magic_Table<pt>::init(eSquare s) {
    BB64 blockers[num_attacks]{}, temp[num_attacks]{};

    mask = rel_occupancy_bits(s);
    shift = 64 - mask.bitcount();
    const U64 n = 1ULL << mask.bitcount();
    for (U64 i = 0; i < n; i++) {
        blockers[i] = blocker_index_mask(mask, i);
        temp[i] = attack_bb(s, blockers[i]);
    }

    for (U64 attempt = 0; attempt < 1000000; attempt++) {
        magic = magic_rng.sparse_rand();
        if (BB64((magic * U64(mask)) & 0xFF00000000000000ULL).bitcount() > 6) continue;
        std::fill_n(attacks, num_attacks, 0);

        bool failed = false;
        for (U64 i = 0; !failed && (i < n); i++) {
            size_t index = blockers_to_index(blockers[i]);
            if (attacks[index].empty())
                attacks[index] = temp[i];
            else if (attacks[index] != temp[i])
                failed = true;
        }
        if (!failed) return; // Success :)
    }
    std::cerr << "FAILED TO GENERATE MAGICS\n";
    assert(false); // Failure :(
}


void init_magics() {
    for (eSquare s = A1; s < NULL_SQUARE; s++) {
        bishop_magics[s].init(s);
        rook_magics[s].init(s);
    }
}

template std::vector<Move>& Move_Generator::gen_moves<WHITE>();
template std::vector<Move>& Move_Generator::gen_moves<BLACK>();

template class Magic_Table<BISHOP>;
template class Magic_Table<ROOK>;