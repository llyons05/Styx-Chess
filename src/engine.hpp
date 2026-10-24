#pragma once

#include "board.hpp"
#include "movegen.hpp"
#include "utils.hpp"

#include <atomic>

constexpr int CAPTURE_VAL = 10;

struct Search_Stack {
    int ply = 0;
    Depth reduction = 0;
    Move current_move = Move();
    BB_State_Info state_info;
};

class Engine {
public:
    bool search_cancelled = false;
    std::atomic_bool external_cancel{false};
    U64 nodes_traversed  = 0;
    TimePoint time_limit = 0;
    TimePoint start_time = 0;

    Engine() {}
    Engine& operator=(const Engine& other);

    Move find_move(Board& board, const Time_Info& tc_info);
    
private:
    Move best_move = Move();

    void prepare_for_search(const Board& board, const Time_Info& tc_info);
    TimePoint get_searchtime(const Board& board, const Time_Info& tc_info);
    void init_stack(Search_Stack* stack_base);
    void set_movescores(const Board& board, Move_Generator& mg, Search_Stack* stack);

    Score search_iterate(Board& board);
    Score search_widen(Board& board, Move_Generator& root_mg, Search_Stack* stack, Depth depth, Score prev_score);
    Score search_root(Board& board, Move_Generator& root_mg, Search_Stack* stack, Score alpha, Score beta, Depth depth);
    Score search(Board& board, Search_Stack* stack, Score alpha, Score beta, Depth depth);
    Score q_search(Board& board, Search_Stack* stack, Score alpha, Score beta);
    
    bool do_move(Board& board, Search_Stack* stack, const Move& move);
    void undo_move(Board& board, Search_Stack* stack, const Move& move);
    
    Score eval(const Board& board);
    Score pawn_eval(const Board& board);

    inline void set_search_cancelled() {
        if ((nodes_traversed & 4095) == 0)
            search_cancelled = (get_time() - start_time > time_limit) || external_cancel.load(std::memory_order::relaxed);
    }

    inline bool in_check(const Board& board, eColor c) {
        return Move_Generator::is_attacked(board, !c, board.bbs.king_sq(c));
    }
};

constexpr BB64 big_center = C3 | D3 | E3 | F3
                          | C4 | D4 | E4 | F4
                          | C5 | D5 | E5 | F5
                          | C6 | D6 | E6 | F6;

constexpr BB64 small_center = D4 | E4 | D5 | E5;

constexpr BB64 edges = ROWS[FILE_A] | ROWS[FILE_B] | COLS[0] | COLS[7];