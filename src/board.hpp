#pragma once

#include "bitboards.hpp"
#include "move.hpp"
#include "utils.hpp"

#include <cstdint>
#include <string>


class Board {
public:
    Bitboards bbs;

    Board() : bbs(), rep_stack{bbs.hash_key}, stack_index(0) {};
    Board(const Board& other) : bbs(other.bbs), rep_stack{bbs.hash_key}, stack_index(0) {}
    Board(std::string fen) : bbs(fen), rep_stack{bbs.hash_key}, stack_index(0) {};

    eColor stm() const {
        return bbs.stm;
    }

    Piece move(const Move& m);
    void undo(const Move& m, const BB_State_Info& state_info);
    bool is_draw();

    void print();
private:
    U64 rep_stack[MAX_GAME_LENGTH];
    int stack_index;
};