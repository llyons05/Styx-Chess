#pragma once

#include "move.hpp"
#include "board.hpp"

#include <cstdint>
#include <vector>

const int MAX_LEGAL_MOVES = 218;

int gen_moves(const Board& board, std::vector<Move>& movelist, uint8_t expected_best);