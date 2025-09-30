#pragma once

#include "utils.hpp"
#include "bitboards.hpp"

#include <cstdint>


class Board {
public:
    Bitboards bbs;

    Board();

    eColor stm() const {
        return bbs.side_to_move;
    }

    void reset();
    void print();
};