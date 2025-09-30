#pragma once

#include "utils.hpp"

struct Move {
    uint8_t id;

    eSquare from;
    eSquare to;

    ePiece piece_from : 3;
    ePiece piece_to : 3;

    eSpecialMoveFlags move_flag : 3;
    bool is_capture : 1;
};