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

    Move find_move(Board& board, TimePoint time_limit);
    
private:
    Move best_move = Move();

    void prepare_for_search(TimePoint time_limit);
    void init_stack(Search_Stack* stack_base);
    void set_movescores(const Board& board, Move_Generator& mg, Search_Stack* stack);

    Score eval(const Board& board);
    Score search_iterate(Board& board);
    Score search_widen(Board& board, Move_Generator& root_mg, Search_Stack* stack, Depth depth, Score prev_score);
    Score search_root(Board& board, Move_Generator& root_mg, Search_Stack* stack, Score alpha, Score beta, Depth depth);
    Score search(Board& board, Search_Stack* stack, Score alpha, Score beta, Depth depth);

    bool do_move(Board& board, Search_Stack* stack, Move& move);

    inline void set_search_cancelled() {
        if ((nodes_traversed & 4095) == 0)
            search_cancelled = (get_time() - start_time > time_limit) || external_cancel.load(std::memory_order::relaxed);
    }

    inline bool in_check(const Board& board, eColor c) {
        return Move_Generator::is_attacked(board, !c, board.bbs.king_sq(c));
    }
};