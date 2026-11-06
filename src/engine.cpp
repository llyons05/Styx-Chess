#include "engine.hpp"

#include "uci.hpp"


Engine& Engine::operator=(Engine&& other) {
    if (this != &other) {
        search_cancelled = other.search_cancelled;
        external_cancel.store(other.external_cancel.load(std::memory_order::relaxed),
                            std::memory_order::relaxed);
        nodes_traversed = other.nodes_traversed;
        time_limit = other.time_limit;
        start_time = other.start_time;
        best_move = other.best_move;
        tt = other.tt;
    }
    return *this;
}

void Engine::new_game() {
    search_cancelled = false;
    external_cancel.store(false, std::memory_order::relaxed);
    nodes_traversed = 0;
    time_limit = INF_TIME;
    start_time = 0;
    best_move = Move();
    tt.clear();
    for (int i = 0; i <= MAX_PLY; i++) {
        killer_moves[i][0] = Move();
        killer_moves[i][1] = Move();
    }
    for (eColor c = WHITE; c < NUM_COLORS; c++)
        for (int i = 0; i < 64; i++)
            for (int j = 0; j < 64; j++)
                history_table[c][i][j] = 0;
}

Move Engine::find_move(Board& board, const Time_Info& tc_info) {
    prepare_for_search(board, tc_info);
    search_iterate(board);
    
    return best_move;
}

void Engine::prepare_for_search(const Board& board, const Time_Info& tc_info) {
    time_limit = get_searchtime(board, tc_info);
    best_move = Move();
    nodes_traversed = 0;
    start_time = get_time();
    search_cancelled = false;

    for (int i = 0; i < NULL_SQUARE; i++)
        for (int j = 0; j < NULL_SQUARE; j++) {
            history_table[WHITE][i][j] /= 2;
            history_table[BLACK][i][j] /= 2;
        }
}

TimePoint Engine::get_searchtime(const Board& board, const Time_Info& tc_info) {
    const TimePoint buffer = 50;
    TimePoint remaining = tc_info.remaining[board.stm()];
    TimePoint inc = tc_info.inc[board.stm()];
    if (remaining > 0) {
        if (tc_info.moves_to_go > 0)
            return (remaining - buffer)/tc_info.moves_to_go + inc;
        return remaining/20 + inc/2;
    }
    return tc_info.movetime;
}

void Engine::init_stack(const Board& board, Search_Stack* stack_base) {
    for (int i = 0; i <= MAX_PLY; i++)
        stack_base[i].ply = i;
    stack_base[0].state_info = board.bbs.state_info();
    stack_base[0].threats[WHITE] = Move_Generator::attacks(board, WHITE);
    stack_base[0].threats[BLACK] = Move_Generator::attacks(board, BLACK);
}

void Engine::update_stack(const Board& board, Search_Stack* stack) {
    stack->threats[!board.stm()] = Move_Generator::attacks(board, !board.stm());
    stack->state_info = board.bbs.state_info();
}

void Engine::set_movescores(Board& board, Move_Generator& mg, Search_Stack* stack, U8 tt_move_id) {
    BB64 opp_threats = stack->threats[!board.stm()];
    for (std::size_t i = 0; i < mg.movelist.size(); i++) {
        Move& move = mg[i];
        if (move.is_capture()) {
            Score see = mat_val(move.piece_captured);
            if (opp_threats.filled(move.to))
                see = static_exchange_eval(board, move);

            if (see < -50)
                move.score = -CAPTURE_ORDERING_VAL + see - move.piece_from;
            else
                move.score = CAPTURE_ORDERING_VAL + see - move.piece_from;
        }

        if (move.is_promo()) {
            move.score += PROMO_ORDERING_VAL + move.piece_to;
        }

        if (move.id == tt_move_id)
            move.score = TT_ORDERING_VAL;
        
        if (move.score != 0)
            continue;

        if (move.score < KILLER_ORDERING_VAL) {
            if (move == killer_moves[stack->ply][0])
                move.score = KILLER_ORDERING_VAL;
            else if (move == killer_moves[stack->ply][1])
                move.score = KILLER_ORDERING_VAL - 1;
        }
        if (move.score < KILLER_ORDERING_VAL-1)
            move.score = history_table[board.stm()][move.from][move.to];
    }
}

eSquare least_val_attacker(const BB64 bbs[NUM_COLORS], BB64 attackers) {
    for (ePieceType p = PAWN; p < NUM_PIECE_TYPES; p++) {
        if (bbs[p] & attackers)
            return (bbs[p] & attackers).lsb();
    }
    return NULL_SQUARE;
}

Score Engine::static_exchange_eval(const Board& board, const Move& move) {
    if (move.piece_from == KING)
        return -mat_val(KING);

    Score gain[32]{};
    int d = 0;
    gain[d] = mat_val(move.piece_captured);
    BB64 occupied = board.bbs.all() & ~BB64(move.from);
    BB64 attacks_to = Move_Generator::attacks_to(board.bbs.piece_bbs, occupied, move.to);
    BB64 causes_xray = (board.bbs[WHITE] ^ board.bbs[WHITE, KING] ^ board.bbs[WHITE, KNIGHT]) | (board.bbs[BLACK] ^ board.bbs[BLACK, KING] ^ board.bbs[BLACK, KNIGHT]);
    eColor curr_color = board.stm();
    eSquare curr_attacker = move.from;
    do {
        d++;
        assert(d < 32);
        ePieceType pt = board.bbs.piece_at(curr_attacker).pt;
        curr_color = !curr_color;
        gain[d] = mat_val(pt) - gain[d-1];
        attacks_to &= ~BB64(curr_attacker);
        occupied &= ~BB64(curr_attacker);
        if (causes_xray.filled(curr_attacker))
            attacks_to = Move_Generator::attacks_to(board.bbs.piece_bbs, occupied, move.to);

        if ((attacks_to != 0) && pt == KING) {
            d++;
            break;
        }
        curr_attacker = least_val_attacker(board.bbs.piece_bbs[curr_color], attacks_to);

        if (std::max(-gain[d-1], gain[d]) < 0) {
            if (curr_attacker != NULL_SQUARE)
                d++;
            break;
        }
    } while (curr_attacker < NULL_SQUARE);

    while (--d)
        gain[d-1] = -std::max(-gain[d-1], gain[d]);
    return gain[0];
}

void Engine::set_killers(const Move& move, Search_Stack* stack) {
    if (!move.is_capture() && !move.is_promo()) {
        if (move != killer_moves[stack->ply][0])
            killer_moves[stack->ply][1] = killer_moves[stack->ply][0];
        killer_moves[stack->ply][0] = move;
    }
}

// Return true if legal move
bool Engine::do_move(Board& board, Search_Stack* stack, const Move& move) {
    board.move(move);
    stack->current_move = move;
    (stack+1)->threats[board.stm()] = Move_Generator::attacks(board, board.stm());

    return !in_check(board, !board.stm(), stack+1);
}


void Engine::undo_move(Board& board, Search_Stack* stack, const Move& move) {
    board.undo(move, stack->state_info);
}

Score Engine::search_iterate(Board& board) {
    Search_Stack stack[MAX_PLY + 1] {};
    init_stack(board, stack);
    Move_Generator root_mg(board);
    set_movescores(board, root_mg, stack, 255);

    Depth current_depth = 1;
    Score final_val = search_root(board, root_mg, stack, -INF_SCORE, INF_SCORE, current_depth++);
    UCI_Command::info(nodes_traversed, 1, start_time, final_val, tt.hashfull());

    while (((abs(final_val) < MAX_SCORE - current_depth)) && !search_cancelled && (current_depth < MAX_PLY)) {
        final_val = search_widen(board, root_mg, stack, current_depth, final_val);
        UCI_Command::info(nodes_traversed, current_depth, start_time, final_val, tt.hashfull());
        current_depth++;
    }

    return final_val;
}

Score Engine::search_widen(Board& board, Move_Generator& root_mg, Search_Stack* stack, Depth depth, Score prev_score) {
    Score window_size = 50;
    Score alpha = std::max(prev_score - window_size, -INF_SCORE);
    Score beta  = std::min(prev_score + window_size, INF_SCORE);
    Score score = prev_score;
    U8 fails = 0;
    while (true) {
        score = search_root(board, root_mg, stack, alpha, beta, depth);
        if (search_cancelled) {
            if (score <= alpha)
                score = prev_score;
            break;
        }

        if (score <= alpha) {
            beta = alpha;
            alpha = std::max(score - window_size, -INF_SCORE);
        }
        else if (score >= beta) {
            alpha = std::max(beta - window_size, alpha);
            beta = std::min(score + window_size, INF_SCORE);
        }
        else
            break;

        if (fails > 2)
            window_size = INF_SCORE;
        else
            window_size += window_size/3;
        fails++;
    }
    return score;
}

Score Engine::search_root(Board& board, Move_Generator& root_mg, Search_Stack* stack, Score alpha, Score beta, Depth depth) {
    nodes_traversed++;
    set_search_cancelled();
    int chosen_move_idx = 0;
    eEntry_Type tt_flag = TT_LOWERBOUND;

    for (size_t i = 0; i < root_mg.movelist.size(); i++) {
        Move& m = root_mg.next_move(i);
        // std::cout << "Playing move " << m << " at depth " << depth << " score=" << m.score << "\n";
        if (!do_move(board, stack, m)) {
            undo_move(board, stack, m);
            m.score = -INF_SCORE;
            continue;
        }
        update_stack(board, stack+1);
        Score val = -search(board, stack+1, -beta, -alpha, depth - 1);
        undo_move(board, stack, m);

        if (search_cancelled)
            break;

        // std::cout << "\tScore: " << val << "\n";
        if (val > alpha) {
            chosen_move_idx = i;
            best_move = m;
            alpha = val;
            if (alpha >= beta) {
                tt_flag = TT_UPPERBOUND;
                break;
            }
            tt_flag = TT_EXACT;
        }
    }
    if (!search_cancelled) {
        assert(alpha > -INF_SCORE);
        tt.store(board.bbs.hash_key, alpha, depth, stack->ply, best_move.id, tt_flag);
    }
    root_mg[chosen_move_idx].score = depth*ROOT_ORDERING_VAL;
    // std::cout << "Best move chosen at depth " << depth << " " << best_move << "\n";
    return alpha;
}

Score Engine::search(Board& board, Search_Stack* stack, Score alpha, Score beta, Depth depth) {
    assert(stack->ply <= MAX_PLY);
    nodes_traversed++;
    set_search_cancelled();
    if (search_cancelled)
        return 0;
    
    bool flag_in_check = in_check(board, board.stm(), stack);
    if (flag_in_check) depth++;

    if (board.is_draw()) {
        return 0;
    }

    TT_Entry& tt_entry = tt.at(board.bbs.hash_key);
    U8 tt_move_id = 255;
    if (tt_entry.hash == board.bbs.hash_key) {
        tt_move_id = tt_entry.best_move_id;
        if (tt_entry.depth >= depth) {
            Score tt_eval = tt_entry.root_relative_eval(stack->ply);
            if (tt_entry.entry_type == TT_EXACT)
                return tt_eval;
            if (tt_entry.entry_type == TT_UPPERBOUND)
                beta = std::min(beta, tt_eval);
            else
                alpha = std::max(alpha, tt_eval);
        }
        if (alpha >= beta)
            return alpha;
    }

    if ((depth <= 0) || (stack->ply >= MAX_PLY))
        return q_search(board, stack, alpha, beta);

    // Mate distance pruning
    Score mate_score = MAX_SCORE - stack->ply;
    if (alpha < -mate_score) alpha = -mate_score;
    if (beta > mate_score - 1) beta = mate_score - 1;
    if (alpha >= beta) return alpha;

    // Null Move pruning
    if (!flag_in_check
     && (depth > 2)
     && !(stack-1)->current_move.is_nullmove()
     && (board.bbs.piece_counts[board.stm()][PAWN]*mat_val(PAWN) < board.bbs.material[board.stm()])
     && (eval(board, stack) >= beta)) {
        Depth reduction = (depth > 5) ? 3 : 2;
        do_move(board, stack, Move::null());
        update_stack(board, stack+1);
        Score null_score = -search(board, stack+1, -beta, -(beta - 1), depth-reduction);
        undo_move(board, stack, Move::null());
        if (null_score >= beta) {
            return beta;
        }
    }

    // Futility Pruning
    bool futility_prune = false;
    if (!flag_in_check
     && (depth <= 3)
     && (abs(alpha) < MIN_WINNING_SCORE)
     && (eval(board, stack) + 200*depth <= alpha))
        futility_prune = true;

    Move_Generator mg(board);
    set_movescores(board, mg, stack, tt_move_id);
    eEntry_Type tt_flag = TT_UPPERBOUND;
    U8 best_move_id = 255;
    int moves_tried = 0;
    for (size_t i = 0; i < mg.movelist.size(); i++) {
        Move& m = mg.next_move(i);
        if (!do_move(board, stack, m)) {
            undo_move(board, stack, m);
            continue;
        }
        update_stack(board, stack+1);

        if (futility_prune
         && moves_tried
         && !m.is_capture()
         && !m.is_promo()
         && !in_check(board, board.stm(), stack+1)) {
            undo_move(board, stack, m);
            continue;
        }

        stack->reduction = 0;
        if ((moves_tried > 2)
         && (m.score < KILLER_ORDERING_VAL-1)
         && !in_check(board, board.stm(), stack+1)) {
            stack->reduction = (moves_tried > 5) ? depth/3 : 1;
            if (flag_in_check) stack->reduction--;
            stack->reduction = std::clamp(stack->reduction, 0, depth);
        }

        moves_tried++;
        Depth new_depth = depth - 1 - stack->reduction;
        Score val = -search(board, stack+1, -beta, -alpha, new_depth);

        if ((val > alpha) && (stack->reduction != 0)) {
            new_depth = depth-1;
            val = -search(board, stack+1, -beta, -alpha, new_depth);
        }

        undo_move(board, stack, m);

        if (search_cancelled)
            break;

        if (val > alpha) {
            alpha = val;
            best_move_id = m.id;
            if (alpha >= beta) {
                tt_flag = TT_LOWERBOUND;
                set_killers(m, stack);
                if (!m.is_capture() && !m.is_promo())
                    history_table[board.stm()][m.from][m.to] << depth*depth;

                break;
            }
            tt_flag = TT_EXACT;
        }
    }
    if (moves_tried == 0) {
        if (flag_in_check)
            return -MAX_SCORE + stack->ply;
		else
            return 0;
    }

    if (!search_cancelled) {
        assert(alpha > -INF_SCORE);
        tt.store(board.bbs.hash_key, alpha, depth, stack->ply, best_move_id, tt_flag);
    }

    return alpha;
}


Score Engine::q_search(Board& board, Search_Stack* stack, Score alpha, Score beta) {
    assert(stack->ply <= MAX_PLY);
    nodes_traversed++;
    set_search_cancelled();
    if (search_cancelled)
        return 0;

    // The only draws that can happen in quiescence are insufficient material (or stalemate, but we check that too)
    if (!(board.bbs.has_mating_material(WHITE) || board.bbs.has_mating_material(BLACK)))
        return 0;

    Score static_eval = eval(board, stack);
    if (stack->ply >= MAX_PLY)
        return static_eval;

    if (static_eval >= beta)
        return beta;
    if (static_eval > alpha)
        alpha = static_eval;
    
    Move_Generator mg(board);
    set_movescores(board, mg, stack, 255);

    int legal_moves = 0;
    for (size_t i = 0; i < mg.movelist.size(); i++) {
        Move& m = mg.next_move(i);
        if (!do_move(board, stack, m)) {
            undo_move(board, stack, m);
            continue;
        }
        legal_moves++;
        if (!(m.is_capture() || m.is_promo()) || (m.score < 0)) {
            undo_move(board, stack, m);
            continue;
        }
        update_stack(board, stack+1);
        Score val = -q_search(board, stack+1, -beta, -alpha);
        undo_move(board, stack, m);
        
        if (search_cancelled)
            break;
        
        if (val > alpha) {
            alpha = val;
            if (val >= beta)
                break;
        }
    }

    if (legal_moves == 0) {
        if (in_check(board, board.stm(), stack))
            return -MAX_SCORE + stack->ply;
		else
            return 0;
    }

    return alpha;
}

Score Engine::eval(const Board& board, Search_Stack* stack) {
    Score result = 0;
    int game_phase = board.bbs.piece_counts[WHITE][BISHOP] + board.bbs.piece_counts[WHITE][KNIGHT] + 2*board.bbs.piece_counts[WHITE][ROOK] + 4*board.bbs.piece_counts[WHITE][QUEEN]
                   + board.bbs.piece_counts[BLACK][BISHOP] + board.bbs.piece_counts[BLACK][KNIGHT] + 2*board.bbs.piece_counts[BLACK][ROOK] + 4*board.bbs.piece_counts[BLACK][QUEEN];
    game_phase = std::min(24, game_phase);
    int midgame_weight = game_phase;
    int endgame_weight = 24 - game_phase;

    Score white_mat = board.bbs.material[WHITE];
    Score black_mat = board.bbs.material[BLACK];

    Score knight_score = ((board.bbs[BLACK, KNIGHT] & edges).bitcount() - (board.bbs[WHITE, KNIGHT] & edges).bitcount()) * 10;

    Score center_score = ((board.bbs[WHITE] & small_center).bitcount() - (board.bbs[BLACK] & small_center).bitcount()) * 10
                       + ((board.bbs[WHITE] & big_center).bitcount() - (board.bbs[BLACK] & big_center).bitcount()) * 5;

    Score midgame_pawn_score = midgame_pawn_eval(board);
    Score endgame_pawn_score = endgame_pawn_eval(board);
    Score king_safety_score = (king_safety_table[board.bbs.king_sq(WHITE)] - king_safety_table[63 - board.bbs.king_sq(BLACK)]) * 2;

    BB64 white_attacks = stack->threats[WHITE] & ~board.bbs[WHITE];
    BB64 black_attacks = stack->threats[BLACK] & ~board.bbs[BLACK];
    Score attack_score = (white_attacks.bitcount() - black_attacks.bitcount()) * 4;
    Score threat_score = ((white_attacks & board.bbs[BLACK]).bitcount() - (black_attacks & board.bbs[WHITE]).bitcount()) * 8;
    Score hanging_score = hanging_pieces_eval(board, stack);

    result += white_mat - black_mat;
    result += threat_score + attack_score + hanging_score + knight_score;

    Score midgame_score = center_score + midgame_pawn_score + king_safety_score;
    Score endgame_score = endgame_pawn_score;

    result += ((midgame_score * midgame_weight) + (endgame_score * endgame_weight)) / 24;

    if (!board.bbs.has_mating_material(WHITE))
        result = std::min(0, result);
    if (!board.bbs.has_mating_material(BLACK))
        result = std::max(0, result);

    if (board.stm() == BLACK)
        return -result;
    return result;
}


Score Engine::endgame_pawn_eval(const Board& board) {
    Score result = 0;
    BB64 white_pawns = board.bbs[WHITE, PAWN];
    while (white_pawns) {
        eSquare s = white_pawns.pop();
        U8 row = square_row(s);
        if (row >= 4)
            result += row - 3;
    }
    BB64 black_pawns = board.bbs[BLACK, PAWN];
    while (black_pawns) {
        eSquare s = black_pawns.pop();
        U8 row = square_row(s);
        if (row <= 3)
            result -= 4 - row;
    }

    return result*5;
}


Score Engine::midgame_pawn_eval(const Board& board) {
    Score result = 0;
    BB64 white_pawns = board.bbs[WHITE, PAWN];
    BB64 black_pawns = board.bbs[BLACK, PAWN];
    U8 white_king_file = square_col(board.bbs.king_sq(WHITE));
    U8 black_king_file = square_col(board.bbs.king_sq(BLACK));
    if (square_row(board.bbs.king_sq(WHITE)) == 0) {
        if (white_king_file > FILE_F) {
            result += (white_pawns & (BB64(F2) | G2 | H2)).bitcount() * 8;
            result += (white_pawns & (BB64(F3) | G3 | H3)).bitcount() * 4;
        }
        else if (white_king_file < FILE_D) {
            result += (white_pawns & (BB64(A2) | B2 | C2)).bitcount() * 8;
            result += (white_pawns & (BB64(A3) | B3 | C3)).bitcount() * 4;
        }
    }
    if (square_row(board.bbs.king_sq(BLACK)) == 7) {
        if (black_king_file > FILE_F) {
            result -= (black_pawns & (BB64(F7) | G7 | H7)).bitcount() * 8;
            result -= (black_pawns & (BB64(F6) | G6 | H6)).bitcount() * 4;
        }
        else if (black_king_file < FILE_D) {
            result -= (black_pawns & (BB64(A7) | B7 | C7)).bitcount() * 8;
            result -= (black_pawns & (BB64(A6) | B6 | C6)).bitcount() * 4;
        }
    }
    return result;
}


Score Engine::hanging_pieces_eval(const Board& board, Search_Stack* stack) {
    BB64 white_hanging = (stack->threats[BLACK] & ~stack->threats[WHITE]) & board.bbs[WHITE];
    BB64 black_hanging = (stack->threats[WHITE] & ~stack->threats[BLACK]) & board.bbs[BLACK];

    Score white_scalar = board.stm() == WHITE ? (white_hanging.bitcount() > 1 ? 8 : 16) : 4;
    Score black_scalar = board.stm() == BLACK ? (black_hanging.bitcount() > 1 ? 8 : 16) : 4;

    Score result = 0;
    while (white_hanging) {
        eSquare s = white_hanging.pop();
        Piece p = board.bbs.piece_at(s);
        result -= mat_val(p.pt) / white_scalar;
    }

    while (black_hanging) {
        eSquare s = black_hanging.pop();
        Piece p = board.bbs.piece_at(s);
        result += mat_val(p.pt) / black_scalar;
    }
    return result;
}