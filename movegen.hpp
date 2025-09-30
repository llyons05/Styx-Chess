#pragma once

#include "board.hpp"

#include <cstdint>
#include <vector>

const int MAX_LEGAL_MOVES = 218;

struct Move {
    uint8_t id;

    eSquare from;
    eSquare to;

    ePiece piece_from : 3;
    ePiece piece_to : 3;

    eSpecialMoveFlags move_flag : 3;
    bool is_capture : 1;
};

int gen_moves(const Board& board, std::vector<Move>& movelist, uint8_t expected_best);