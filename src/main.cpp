#include "board.hpp"
#include "engine.hpp"
#include "movegen.hpp"
#include "uci.hpp"
#include "utils.hpp"

#include <future>
#include <iostream>
#include <string>
#include <syncstream>
#include <vector>

void cli_mode(eColor player_color, const Time_Info& tc_info, std::string fen) {
    Board board(fen);
    Engine engine;
    board.print();
    std::vector<std::pair<Move, BB_State_Info>> move_history;

    while (true) {
        if (board.is_draw())
            break;
        Move_Generator mg(board);
        bool undo = false;
        int legal_movecount = 0;
        for (Move& move : mg.movelist) {
            BB_State_Info info = board.bbs.state_info();
            board.move(move);
            if (!Move_Generator::is_attacked(board, board.stm(), board.bbs.king_sq(!board.stm())))
                legal_movecount++;
            board.undo(move, info);
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
                        BB_State_Info info = board.bbs.state_info();
                        board.move(move);
                        if (!Move_Generator::is_attacked(board, board.stm(), board.bbs.king_sq(!board.stm())))
                            legal_moves.push_back(move);
                        board.undo(move, info);
                    }
                }
                if (legal_moves.empty())
                    std::cout << get_colored("No legal moves to " + square_str(square) + "\n", 31);
            }
            if (undo) {
                if (!move_history.empty()) {
                    board.undo(move_history.back().first, move_history.back().second);
                    move_history.pop_back();
                    if (!move_history.empty()) {
                        board.undo(move_history.back().first, move_history.back().second);
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
            m = engine.find_move(board, tc_info);
        }
        BB_State_Info info = board.bbs.state_info();
        board.move(m);
        move_history.push_back({m, info});
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

int engine_go(Board& board, Engine& engine, const Time_Info& tc_info) {
    Move best_move = engine.find_move(board, tc_info);
    UCI_Command::bestmove(best_move);
    return 0;
}

void comp_mode() {
    Board board;
    Engine engine;
    std::future<int> engine_future;
    while (true) {
        std::string line;
        if (!std::getline(std::cin, line) || line.starts_with("quit"))
            break;
        if (engine_future.valid() && (engine_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready))
            engine_future.get();
        
        const UCI_Command com(line, board, engine, engine_future.valid());

        if (com.engine_go) {
            engine_future = std::async(std::launch::async, engine_go, std::ref(board), std::ref(engine), std::ref(com.tc_info));
        }

        if (com.engine_quit) {
            engine_future.get();
            exit(0);
        }
    }
}

int main(int argc, char* argv[]) {
    if ((argc >= 2) && (std::string(argv[1]) == "-c")) {
        Time_Info tc_info;
        eColor player_color = ((argc >= 3) && (std::string(argv[2]) == "w")) ? WHITE : BLACK;
        tc_info.movetime = (argc >= 4) ? std::stoll(argv[3]) : 1000;
        std::string fen = (argc >= 5) ? argv[4] : "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -";
        cli_mode(player_color, tc_info, fen);
    }
    else {
        comp_mode();
    }
}