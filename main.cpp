#include "utils.hpp"
#include "board.hpp"
#include "movegen.hpp"

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

    Move_Generator mg(board, 0);
    mg.gen_moves();
    board.move(move);
    board.print();
}