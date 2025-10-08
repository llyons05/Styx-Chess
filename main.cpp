#include "utils.hpp"
#include "board.hpp"

#include <iostream>

int main() {
    Board board;
    board.print();

    Move move;
    move.id = 0;
    move.from = A2;
    move.to = A4;
    move.piece_from = PAWN;
    move.piece_to = PAWN;

    board.bbs.handle_move(move);
    board.print();
}