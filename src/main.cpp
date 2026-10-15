#include "utils.hpp"
#include "board.hpp"
#include "engine.hpp"
#include "movegen.hpp"

#include <iostream>
#include <string>
#include <vector>

int main(int argc, char* argv[]) {
    std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -";
    if (argc >= 2)
        fen = argv[1];

    Board board(fen);
    Engine engine;
    board.print();
    std::vector<Move> move_history;
    eColor player_color = WHITE;
    TimePoint time_limit = 3000;

    while (true) {
        Move_Generator mg(board);
        bool undo = false;
        int legal_movecount = 0;
        for (Move& move : mg.movelist) {
            board.move(move);
            if (!Move_Generator::is_attacked(board, board.stm(), board.bbs.king_sq(!board.stm())))
                legal_movecount++;
            board.undo(move);
        }
        if (legal_movecount == 0) {
            std::cout << "GAME OVER!\n";
            break;
        }
        Move m;
        if (board.stm() == player_color) {
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
                        board.move(move);
                        if (!Move_Generator::is_attacked(board, board.stm(), board.bbs.king_sq(!board.stm())))
                            legal_moves.push_back(move);
                        board.undo(move);
                    }
                }
                if (legal_moves.empty())
                    std::cout << get_colored("No legal moves to " + square_str(square) + "\n", 31);
            }
            if (undo) {
                if (!move_history.empty()) {
                    board.undo(move_history.back());
                    move_history.pop_back();
                    if (!move_history.empty()) {
                        board.undo(move_history.back());
                        move_history.pop_back();
                    }
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
            m = legal_moves[move_index];
        }
        else {
            m = engine.find_move(board, time_limit);
        }
        board.move(m);
        move_history.push_back(m);
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