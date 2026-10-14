#pragma once

#include "utils.hpp"
#include "bitboards.hpp"
#include "move.hpp"

#include <cstdint>
#include <string>


class Board {
public:
    Bitboards bbs;

    Board() : bbs() {};
    Board(std::string fen) : bbs(fen) {};

    eColor stm() const {
        return bbs.stm;
    }

    Piece move(const Move& m);
    void undo(const Move& m);

    void print();
};