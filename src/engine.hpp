#pragma once

#include "board.hpp"
#include "movegen.hpp"
#include "utils.hpp"

#include <atomic>

struct Search_Stack {
    int ply = 0;
    Depth reduction = 0;
    Move current_move = Move();
    BB_State_Info state_info;

    BB64 threats[NUM_COLORS]{};
};

class Engine {
public:
    bool search_cancelled = false;
    std::atomic_bool external_cancel{false};
    U64 nodes_traversed  = 0;
    TimePoint time_limit = 0;
    TimePoint start_time = 0;

    Engine(U64 tt_size) : tt(tt_size) {}
    Engine& operator=(Engine&& other);
    void new_game();

    Move find_move(Board& board, const Time_Info& tc_info);
    
private:
    Transposition_Table tt;
    Move killer_moves[MAX_PLY+1][2]{};
    Clamped_Stat<int16_t, 998> history_table[NUM_COLORS][64][64]{};
    Move best_move = Move();

    void prepare_for_search(const Board& board, const Time_Info& tc_info);
    TimePoint get_searchtime(const Board& board, const Time_Info& tc_info);
    void init_stack(const Board& board, Search_Stack* stack_base);
    void update_stack(const Board& board, Search_Stack* stack);
    void set_movescores(const Board& board, Move_Generator& mg, Search_Stack* stack, U8 tt_move_id);
    void set_killers(const Move& move, Search_Stack* stack);

    Score search_iterate(Board& board);
    Score search_widen(Board& board, Move_Generator& root_mg, Search_Stack* stack, Depth depth, Score prev_score);
    Score search_root(Board& board, Move_Generator& root_mg, Search_Stack* stack, Score alpha, Score beta, Depth depth);
    Score search(Board& board, Search_Stack* stack, Score alpha, Score beta, Depth depth);
    Score q_search(Board& board, Search_Stack* stack, Score alpha, Score beta);
    
    bool do_move(Board& board, Search_Stack* stack, const Move& move);
    void undo_move(Board& board, Search_Stack* stack, const Move& move);
    
    Score eval(const Board& board, Search_Stack* stack);
    Score endgame_pawn_eval(const Board& board);
    Score midgame_pawn_eval(const Board& board);

    inline void set_search_cancelled() {
        if ((nodes_traversed & 4095) == 0)
            search_cancelled = (get_time() - start_time > time_limit) || external_cancel.load(std::memory_order::relaxed);
    }

    inline bool in_check(const Board& board, eColor c, Search_Stack* stack) {
        return !(stack->threats[!c] & board.bbs.king_sq(c)).empty();
    }
};

constexpr BB64 big_center = BB64(C3) | D3 | E3 | F3
                          | C4 | D4 | E4 | F4
                          | C5 | D5 | E5 | F5
                          | C6 | D6 | E6 | F6;

constexpr BB64 small_center = BB64(D4) | E4 | D5 | E5;

constexpr BB64 edges = ROWS[FILE_A] | ROWS[FILE_H] | COLS[0] | COLS[7];

constexpr Score king_safety_table[64] {
     8,  8,  5,  2,  2,  5,  8,  8,
     4,  3,  0,  0,  0,  0,  3,  4,
     0,  0, -3, -3, -3, -3,  0,  0,
    -3, -5, -5, -5, -5, -5, -5, -3,
    -5, -5, -6, -7, -7, -6, -5, -5,
    -7, -7, -7, -7, -7, -7, -7, -7,
    -8, -8, -8, -8, -8, -8, -8, -8,
    -8, -8, -8, -8, -8, -8, -8, -8,
};

constexpr int TT_ORDERING_VAL       = 100000;
constexpr int ROOT_ORDERING_VAL     = 100000;
constexpr int PROMO_ORDERING_VAL    = 5000;
constexpr int CAPTURE_ORDERING_VAL  = 3000;
constexpr int KILLER_ORDERING_VAL   = 1000;