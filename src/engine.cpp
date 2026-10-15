#include "engine.hpp"

Move Engine::find_move(Board& board, TimePoint time_limit) {
    prepare_for_search(time_limit);
    Score val = search_iterate(board);

    std::cout << "Best move was " << best_move << " with value of " << val << "\n";
    
    return best_move;
}

void Engine::prepare_for_search(TimePoint time_limit) {
    this->time_limit = time_limit;
    best_move = Move();
    nodes_traversed = 0;
    start_time = get_time();
    search_cancelled = false;
}

void Engine::init_stack(Search_Stack* stack_base) {
    for (int i = 0; i <= MAX_PLY; i++)
        stack_base[i].ply = i;
}

// Return true if legal move
bool Engine::do_move(Board& board, Search_Stack* stack, Move& move) {
    board.move(move);
    stack->current_move = move;

    return !in_check(board, !board.stm());
}

Score Engine::search_iterate(Board& board) {
    Search_Stack stack[MAX_PLY + 1] {};
    init_stack(stack);
    Depth current_depth = 1;
    Score final_val = search_root(board, stack, -INF_SCORE, INF_SCORE, current_depth++);

    while ((abs(final_val) < MIN_WINNING_SCORE) && !search_cancelled && (current_depth < MAX_PLY)) {
        Score temp_val = search_widen(board, stack, current_depth, final_val);
        if (!search_cancelled)
            final_val = temp_val;
        current_depth++;
    }

    std::cout << "\nReached depth " << current_depth - 1 << ", searched " << nodes_traversed << " nodes\n";

    return final_val;
}

Score Engine::search_widen(Board& board, Search_Stack* stack, Depth depth, Score prev_score) {
    Score window_size = 500;
    Score alpha = std::max(prev_score - window_size, -INF_SCORE);
    Score beta  = std::min(prev_score + window_size, INF_SCORE);
    Score score = prev_score;
    Move prev_best = best_move;
    U8 fails = 0;
    while (true) {
        score = search_root(board, stack, alpha, beta, depth);
        if (search_cancelled) {
            score = prev_score;
            best_move = prev_best;
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

Score Engine::search_root(Board& board, Search_Stack* stack, Score alpha, Score beta, Depth depth) {
    nodes_traversed++;
    set_search_cancelled();
    Move_Generator mg(board);
    Move chosen_move = mg[0];
    Score best_val = -INF_SCORE;

    for (Move& m : mg.movelist) {
        // std::cout << "Playing move " << m << " at depth " << depth << "\n";
        if (!do_move(board, stack, m)) {
            board.undo(m);
            continue;
        }
        
        Score move_score = -search(board, stack+1, -beta, -alpha, depth - 1);
        board.undo(m);
        
        best_val = std::max(move_score, best_val);

        if (search_cancelled)
            break;

        if (move_score > alpha) {
            chosen_move = m;
            alpha = move_score;
            // std::cout << "Move was best\n";
            if (alpha >= beta) {
                break;
            }
        }
    }

    if (!search_cancelled) {
        assert(abs(best_val) < INF_SCORE);
        best_move = chosen_move;
        // std::cout << "Best move chosen at depth " << depth << " " << chosen_move << "\n";
    }
    return best_val;
}

Score Engine::search(Board& board, Search_Stack* stack, Score alpha, Score beta, Depth depth) {
    nodes_traversed++;
    set_search_cancelled();
    if (search_cancelled)
        return 0;

    bool flag_in_check = in_check(board, board.stm());

    if (depth <= 0)
        return eval(board);

    Move_Generator mg(board);
    Score best_val = -INF_SCORE;
    int moves_tried = 0;
    for (Move& m : mg.movelist) {
        if (!do_move(board, stack, m)) {
            board.undo(m);
            continue;
        }
        moves_tried++;
        Score val = -search(board, stack+1, -beta, -alpha, depth - 1);
        board.undo(m);

        if (search_cancelled)
            break;

        best_val = std::max(val, best_val);
        if (val > alpha) {
            alpha = val;
            if (alpha >= beta) {
                break;
            }
        }
    }
    if (moves_tried == 0) {
        if (flag_in_check)
            return -MAX_SCORE + stack->ply;
		else
            return 0;
    }

    return best_val;
}

Score Engine::eval(const Board& board) {
    Score result = 0;
    result += board.bbs[WHITE, PAWN].bitcount() * 100;
    result += board.bbs[WHITE, BISHOP].bitcount() * 300;
    result += board.bbs[WHITE, KNIGHT].bitcount() * 300;
    result += board.bbs[WHITE, ROOK].bitcount() * 500;
    result += board.bbs[WHITE, QUEEN].bitcount() * 800;
    
    result -= board.bbs[BLACK, PAWN].bitcount() * 100;
    result -= board.bbs[BLACK, BISHOP].bitcount() * 300;
    result -= board.bbs[BLACK, KNIGHT].bitcount() * 300;
    result -= board.bbs[BLACK, ROOK].bitcount() * 500;
    result -= board.bbs[BLACK, QUEEN].bitcount() * 800;

    if (board.stm() == BLACK)
        return -result;
    return result;
}