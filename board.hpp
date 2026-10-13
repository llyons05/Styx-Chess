#pragma once

#include "utils.hpp"
#include "bitboards.hpp"
#include "move.hpp"

#include <cstdint>


class Board {
public:
    Bitboards bbs;

    Board();

    eColor stm() const {
        return bbs.stm;
    }

    Piece move(Move& m);

    void reset();
    void print();
};