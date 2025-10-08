#pragma once

#include "utils.hpp"

class Move {
public:
    uint8_t id;

    eSquare from;
    eSquare to;

    ePieceType piece_from : 3;
    ePieceType piece_to : 3;

    eSpecialMoveFlags move_flag : 3;
    bool is_capture : 1;

    Move()
};