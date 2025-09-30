#pragma once

#include "utils.hpp"
#include "bitboards.hpp"

#include <cstdint>


class Board {
    Bitboards bitboards;

    Board();

    void reset();

    void print();
};