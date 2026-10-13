#include "utils.hpp"
#include "board.hpp"
#include "movegen.hpp"

#include <iostream>
#include <string>
#include <vector>

int main() {
    Board board;
    board.print();
    std::vector<std::pair<Move, Piece>> move_history;

    while (true) {
        bool undo = false;
        Move_Generator mg(board);
        int legal_movecount = 0;
        for (Move& move : mg.movelist) {
            Piece p = board.move(move);
            if (!Move_Generator::is_attacked(board, board.stm(), board.bbs.king_sq(!board.stm())))
                legal_movecount++;
            board.undo(move, p);
        }
        if (legal_movecount == 0) {
            std::cout << "GAME OVER!\n";
            break;
        }
        eSquare square = NULL_SQUARE;
        std::vector<Move> legal_moves;
        while (legal_moves.empty()) {
            legal_moves.clear();
            square = get_valid_square();
            if (square == NULL_SQUARE) {
                undo = true;
                break;
            }
            for (Move& move : mg.movelist) {
                if (move.to == square) {
                    Piece p = board.move(move);
                    if (!Move_Generator::is_attacked(board, board.stm(), board.bbs.king_sq(!board.stm())))
                        legal_moves.push_back(move);
                    board.undo(move, p);
                }
            }
            if (legal_moves.empty())
                std::cout << get_colored("No legal moves to " + square_str(square) + "\n", 31);
        }
        if (undo) {
            if (!move_history.empty()) {
                board.undo(move_history.back().first, move_history.back().second);
                move_history.pop_back();
                board.print();
            }
            continue;
        }
        int move_index = 0;
        if (legal_moves.size() > 1) {
            for (size_t i = 0; i < legal_moves.size(); i++) {
                std::cout << i << ": " << legal_moves[i] << ", ";
            }
            std::cout << "\n";
            move_index = get_bounded_num<int>("Type move id: ", 0, legal_moves.size());
        }
        Move m = legal_moves[move_index];
        move_history.push_back({m, board.move(m)});
        board.print();
    }

    if (Move_Generator::is_attacked(board, !board.stm(), board.bbs.king_sq(board.stm()))) {
        if (board.stm() == WHITE)
            std::cout << "BLACK WINS\n";
        else
            std::cout << "WHITE WINS\n";
    }
    else
        std::cout << "STALEMATE\n";
}