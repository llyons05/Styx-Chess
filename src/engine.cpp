#include "engine.hpp"

#include "uci.hpp"

#include <math.h>

const std::array<int, MAX_MOVES> reductions = [] {
    std::array<int, MAX_MOVES> table{};
    for (size_t i = 1; i < table.size(); ++i)
        table[i] = int(512 * sqrt(float(i)));
    return table;
}();

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
    for (eColor c = WHITE; c < NUM_COLORS; c++) {
        for (int i = 0; i < 64; i++)
            for (int j = 0; j < 64; j++)
                history_table[c][i][j] = 0;
    
        for (int i = 0; i < NUM_PIECE_TYPES; i++)
            for (int j = 0; j < 64; j++)
                for (int k = 0; k < NUM_PIECE_TYPES; k++)
                    cap_history_table[c][i][j][k] /= 2;
    }
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
    for (int i = 0; i < NUM_PIECE_TYPES; i++)
        for (int j = 0; j < 64; j++)
            for (int k = 0; k < NUM_PIECE_TYPES; k++) {
                cap_history_table[WHITE][i][j][k] /= 2;
                cap_history_table[BLACK][i][j][k] /= 2;
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
    for (int i = 0; i <= MAX_PLY+1; i++)
        stack_base[i].ply = i-1;
    stack_base[0].psuedo_legal_movecount = 50;
    stack_base[1].state_info = board.bbs.state_info();
    stack_base[1].threats[WHITE] = Move_Generator::attacks(board, WHITE);
    stack_base[1].threats[BLACK] = Move_Generator::attacks(board, BLACK);
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

            int cap_hist = cap_history_table[board.stm()][move.piece_from][move.to][move.piece_captured];
            if (see >= -cap_hist/18)
                move.score = CAPTURE_ORDERING_VAL + mat_val(move.piece_captured)*6 + cap_hist;
            else
                move.score = -CAPTURE_ORDERING_VAL + mat_val(move.piece_captured)*6 + cap_hist;
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
void Engine::update_history(const Board& board, const Move& m, Move_Generator& mg, size_t i, Depth d) {
    if (!m.is_capture() && !m.is_promo()) {
        history_table[board.stm()][m.from][m.to] << d*d;
        for (size_t j = 0; j < i; j++)
            if (!(mg[j].is_capture() || mg[j].is_promo()))
                history_table[board.stm()][mg[j].from][mg[j].to] << -d*d;
    }
    else if (m.is_capture()) {
        cap_history_table[board.stm()][m.piece_from][m.to][m.piece_captured] << d*d;
        for (size_t j = 0; j < i; j++)
            if (mg[j].is_capture())
                cap_history_table[board.stm()][mg[j].piece_from][mg[j].to][mg[j].piece_captured] << -d*d;
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
    Search_Stack stack[MAX_PLY + 2] {};
    init_stack(board, stack);
    Move_Generator root_mg(board, 50);
    set_movescores(board, root_mg, stack+1, 255);
    (stack+1)->psuedo_legal_movecount = root_mg.movelist.size();

    Depth current_depth = 1;
    Score final_val = search_root(board, root_mg, stack+1, -INF_SCORE, INF_SCORE, current_depth++);
    UCI_Command::info(nodes_traversed, 1, start_time, final_val, tt.hashfull());

    while (((abs(final_val) < MAX_SCORE - current_depth)) && !search_cancelled && (current_depth < MAX_PLY)) {
        final_val = search_widen(board, root_mg, stack+1, current_depth, final_val);
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

    Move_Generator mg(board, (stack-2)->psuedo_legal_movecount);
    set_movescores(board, mg, stack, tt_move_id);
    stack->psuedo_legal_movecount = mg.movelist.size();
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
        if ((depth >= 2)
         && (moves_tried > 3)
         && (m.score < KILLER_ORDERING_VAL-1)
         && !in_check(board, board.stm(), stack+1)
         && !flag_in_check) {
            int r = reductions[depth-1] + reductions[moves_tried-1];
            if (m.is_capture())
                r -= mat_val(m.piece_captured)*8;
            else
                r -= m.score/2;
            stack->reduction = std::clamp(r/1024, 0, depth);
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
                update_history(board, m, mg, i, depth);
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
    
    Move_Generator mg(board, (stack-2)->psuedo_legal_movecount);
    set_movescores(board, mg, stack, 255);
    stack->psuedo_legal_movecount = mg.movelist.size();

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
    Eval_Data& e = stack->eval_data;
    e.reset();
    e.game_phase = board.bbs.piece_counts[WHITE][BISHOP] + board.bbs.piece_counts[WHITE][KNIGHT] + 2*board.bbs.piece_counts[WHITE][ROOK] + 4*board.bbs.piece_counts[WHITE][QUEEN]
                 + board.bbs.piece_counts[BLACK][BISHOP] + board.bbs.piece_counts[BLACK][KNIGHT] + 2*board.bbs.piece_counts[BLACK][ROOK] + 4*board.bbs.piece_counts[BLACK][QUEEN];
    e.game_phase = std::min(24, e.game_phase);
    int midgame_weight = e.game_phase;
    int endgame_weight = 24 - e.game_phase;

    e.hanging_pieces[WHITE] = (stack->threats[BLACK] & ~stack->threats[WHITE]) & board.bbs[WHITE];
    e.hanging_pieces[BLACK] = (stack->threats[WHITE] & ~stack->threats[BLACK]) & board.bbs[BLACK];

    e.final_score += board.bbs.material[WHITE] - board.bbs.material[BLACK];
    e.midgame_score += ((board.bbs[WHITE] & small_center).bitcount() - (board.bbs[BLACK] & small_center).bitcount()) * 10
                     + ((board.bbs[WHITE] & big_center).bitcount() - (board.bbs[BLACK] & big_center).bitcount()) * 5;

    pawn_eval(board, stack);
    king_eval(board, stack);
    attack_score(board, stack);

    e.final_score += ((e.midgame_score * midgame_weight) + (e.endgame_score * endgame_weight)) / 24;
    e.final_score += hanging_pieces_eval(board, stack);

    if (!board.bbs.has_mating_material(WHITE))
        e.final_score = std::min(0, e.final_score);
    if (!board.bbs.has_mating_material(BLACK))
        e.final_score = std::max(0, e.final_score);

    if (board.stm() == BLACK)
        e.final_score *= -1;

    return e.final_score;
}

void Engine::attack_score(const Board& board, Search_Stack* stack) {
    BB64 piece_attacks[NUM_COLORS][NUM_PIECE_TYPES]{};
    piece_attacks[WHITE][PAWN]   = Move_Generator::pawn_attacks<WHITE>(board.bbs[WHITE, PAWN]);
    piece_attacks[WHITE][KNIGHT] = Move_Generator::knight_attacks(board.bbs[WHITE, KNIGHT]);
    piece_attacks[WHITE][BISHOP] = Move_Generator::bishop_attacks(board.bbs[WHITE, BISHOP], board.bbs.all());
    piece_attacks[WHITE][ROOK]   = Move_Generator::rook_attacks(board.bbs[WHITE, ROOK], board.bbs.all());
    piece_attacks[WHITE][QUEEN]  = Move_Generator::queen_attacks(board.bbs[WHITE, QUEEN], board.bbs.all());

    piece_attacks[BLACK][PAWN]   = Move_Generator::pawn_attacks<BLACK>(board.bbs[BLACK, PAWN]);
    piece_attacks[BLACK][KNIGHT] = Move_Generator::knight_attacks(board.bbs[BLACK, KNIGHT]);
    piece_attacks[BLACK][BISHOP] = Move_Generator::bishop_attacks(board.bbs[BLACK, BISHOP], board.bbs.all());
    piece_attacks[BLACK][ROOK]   = Move_Generator::rook_attacks(board.bbs[BLACK, ROOK], board.bbs.all());
    piece_attacks[BLACK][QUEEN]  = Move_Generator::queen_attacks(board.bbs[BLACK, QUEEN], board.bbs.all());

    constexpr Score midgame_mobility_value[NUM_PIECE_TYPES] = {2, 4, 3, 3, 1, 0};
    constexpr Score endgame_mobility_value[NUM_PIECE_TYPES] = {2, 3, 3, 3, 2, 0};
    constexpr Score tropism_value[NUM_PIECE_TYPES]          = {5, 4, 4, 5, 6, 0};
    Score pawn_control = piece_attacks[WHITE][PAWN].bitcount() - piece_attacks[BLACK][PAWN].bitcount();
    Score mid_control_score = pawn_control * midgame_mobility_value[PAWN];
    Score end_control_score = pawn_control * endgame_mobility_value[PAWN];
    Score threat_score = 0;
    for (ePieceType pt = PAWN; pt < KING; pt++) {
        Score control = 0;
        if (pt > PAWN)
            control = (piece_attacks[WHITE][pt] & ~piece_attacks[BLACK][PAWN] & ~board.bbs[WHITE]).bitcount()
                    - (piece_attacks[BLACK][pt] & ~piece_attacks[WHITE][PAWN] & ~board.bbs[BLACK]).bitcount();

        BB64 white_threatening = piece_attacks[WHITE][pt] & board.bbs[BLACK];
        BB64 black_threatening = piece_attacks[BLACK][pt] & board.bbs[WHITE];
        while (white_threatening) {
            ePieceType threat_pt = board.bbs.piece_at(white_threatening.pop()).pt;
            threat_score += 1000 + (mat_val(threat_pt) - mat_val(pt));
        }
        while (black_threatening) {
            ePieceType threat_pt = board.bbs.piece_at(black_threatening.pop()).pt;
            threat_score -= 1000 + (mat_val(threat_pt) - mat_val(pt));
        }

        mid_control_score += control * midgame_mobility_value[pt];
        end_control_score += control * endgame_mobility_value[pt];
        stack->eval_data.final_score += ((piece_attacks[WHITE][pt] & piece_attacks[BLACK][KING]).bitcount()
                                      -  (piece_attacks[BLACK][pt] & piece_attacks[WHITE][KING]).bitcount()) * tropism_value[pt];
        stack->eval_data.final_score += ((piece_attacks[WHITE][pt] & piece_attacks[WHITE][KING]).bitcount()
                                      -  (piece_attacks[BLACK][pt] & piece_attacks[BLACK][KING]).bitcount()) * 4;
    }
    stack->eval_data.midgame_score += mid_control_score * 2;
    stack->eval_data.endgame_score += end_control_score * 2;
    stack->eval_data.final_score += threat_score/100;
}

void Engine::pawn_eval(const Board& board, Search_Stack* stack) {
    BB64 white_pawns = board.bbs[WHITE, PAWN];
    BB64 black_pawns = board.bbs[BLACK, PAWN];

    BB64 white_passed = black_pawns.shift<SOUTH>().smear<SOUTH>();
    white_passed = ~(white_passed | white_passed.shift<EAST>() | white_passed.shift<WEST>());

    BB64 black_passed = white_pawns.shift<NORTH>().smear<NORTH>();
    black_passed = ~(black_passed | black_passed.shift<EAST>() | black_passed.shift<WEST>());

    BB64 white_attacks = Move_Generator::pawn_attacks<WHITE>(white_pawns);
    BB64 black_attacks = Move_Generator::pawn_attacks<BLACK>(black_pawns);

    while (white_pawns) {
        eSquare s = white_pawns.pop();
        U8 row = square_row(s);
        if ((COLS[square_col(s)] & white_attacks).empty()) // Isolated Pawns
            stack->eval_data.final_score -= big_center.filled(s) ? 8 : 6;
        if (white_passed.filled(s))
            stack->eval_data.endgame_score += row * 15;
        else if (row > 3)
            stack->eval_data.endgame_score += (row - 3) * 5;
    }
    while (black_pawns) {
        eSquare s = black_pawns.pop();
        U8 row = square_row(s);
        if ((COLS[square_col(s)] & black_attacks).empty())
            stack->eval_data.final_score += big_center.filled(s) ? 8 : 6;
        if (black_passed.filled(s))
            stack->eval_data.endgame_score -= (7 - row) * 15;
        else if (row < 4)
            stack->eval_data.endgame_score -= (4 - row) * 5;
    }
}

void Engine::king_eval(const Board& board, Search_Stack* stack) {
    stack->eval_data.midgame_score += (king_safety_table[board.bbs.king_sq(WHITE)] - king_safety_table[63 - board.bbs.king_sq(BLACK)]) * 2;
    stack->eval_data.midgame_score += king_shield(board);
}


Score Engine::king_shield(const Board& board) {
    constexpr Score shield_1 = 16;
    constexpr Score shield_2 = 12;
    constexpr Score open_file = 12;
    Score white_result = 0;
    Score black_result = 0;
    BB64 white_pawns = board.bbs[WHITE, PAWN];
    BB64 black_pawns = board.bbs[BLACK, PAWN];
    U8 white_king_file = square_col(board.bbs.king_sq(WHITE));
    U8 black_king_file = square_col(board.bbs.king_sq(BLACK));
    if (square_row(board.bbs.king_sq(WHITE)) == 0) {
        if (white_king_file > FILE_F) {
            white_result += (white_pawns & (BB64(F2) | G2 | H2)).bitcount() * shield_1;
            white_result += (white_pawns & (BB64(F3) | G3 | H3)).bitcount() * shield_2;
            for (int c = FILE_H; c >= FILE_F; c--)
                if ((white_pawns & COLS[c] & (ROWS[1] | ROWS[2])).empty()) {
                    white_result -= open_file;
                }
        }
        else if (white_king_file < FILE_D) {
            white_result += (white_pawns & (BB64(A2) | B2 | C2)).bitcount() * shield_1;
            white_result += (white_pawns & (BB64(A3) | B3 | C3)).bitcount() * shield_2;
            for (int c = FILE_A; c <= FILE_C; c++)
                if ((white_pawns & COLS[c] & (ROWS[1] | ROWS[2])).empty())
                    white_result -= open_file;
        }
    }
    if (square_row(board.bbs.king_sq(BLACK)) == 7) {
        if (black_king_file > FILE_F) {
            black_result += (black_pawns & (BB64(F7) | G7 | H7)).bitcount() * shield_1;
            black_result += (black_pawns & (BB64(F6) | G6 | H6)).bitcount() * shield_2;
            for (int c = FILE_H; c >= FILE_F; c--)
                if ((black_pawns & COLS[c] & (ROWS[6] | ROWS[5])).empty())
                    black_result -= open_file;
        }
        else if (black_king_file < FILE_D) {
            black_result += (black_pawns & (BB64(A7) | B7 | C7)).bitcount() * shield_1;
            black_result += (black_pawns & (BB64(A6) | B6 | C6)).bitcount() * shield_2;
            for (int c = FILE_A; c <= FILE_C; c++)
                if ((black_pawns & COLS[c] & (ROWS[6] | ROWS[5])).empty())
                    black_result -= open_file;
        }
    }

    Score result = (white_result*(board.bbs.material[BLACK] - mat_val(KING) - board.bbs.piece_counts[BLACK][PAWN] * mat_val(PAWN)) 
                  - black_result*(board.bbs.material[WHITE] - mat_val(KING) - board.bbs.piece_counts[WHITE][PAWN] * mat_val(PAWN))) / (starting_material - 8*mat_val(PAWN));
    return result;
}

Score Engine::hanging_pieces_eval(const Board& board, Search_Stack* stack) {
    BB64 white_hanging = stack->eval_data.hanging_pieces[WHITE];
    BB64 black_hanging = stack->eval_data.hanging_pieces[BLACK];

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