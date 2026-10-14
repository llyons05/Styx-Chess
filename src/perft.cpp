#include "utils.hpp"
#include "board.hpp"
#include "movegen.hpp"

#include <iostream>
#include <iomanip>
#include <map>
#include <string>
#include <vector>

// const std::map<std::string, std::vector<U64>> expected_values = {
//     {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -", {1, 20, 400, 8902, 197281, 4865609, 119060324}},
//     {"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -", {1, 48, 2039, 97862, 4085603, 193690690, 8031647685}},
//     {"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -", {1, 14, 191, 2812, 43238, 674624, 11030083}},
//     {"r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq -", {1, 6, 264, 9467, 422333, 15833292, 706045033}},
//     {"rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ -", {1, 44, 1486, 62379, 2103487, 89941194}},
//     {"r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - -", {1, 46, 2079, 89890, 3894594, 164075551, 6923051137}}
// };

U64 total_nodes = 0;
U64 captures = 0;
U64 en_passant = 0;
U64 castles = 0;
U64 promotions = 0;

bool failed = false;

U64 perft(Board& board, int depth) {
    total_nodes++;
    U64 result = 0;
    Move_Generator mg(board);

    if (depth <= 0)
        return 1;

    int legal_moves = 0;
    for (const Move& move : mg.movelist) {
        board.move(move);
        if (!Move_Generator::is_attacked(board, board.stm(), board.bbs.king_sq(!board.stm()))) {
            if (depth == 1) {
                captures += move.is_capture();
                en_passant += move.is_ep();
                castles += move.is_castle();
                promotions += move.is_promo();
            }
            result += perft(board, depth - 1);
            legal_moves++;
        }
        board.undo(move);
    }
    return result;
}

void print_results(int depth, U64 leaf_nodes, U64 expected, TimePoint test_duration) {
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "Completed in " << double(test_duration)/1000 << " seconds\n";
    std::cout << "Perft results at depth " << depth << ": " << leaf_nodes << "\n";
    if (expected == leaf_nodes) {
        std::cout << get_colored("Test produced expected results.\n", 32);
    }
    else {
        std::cout << get_colored("Test produced incorrect results", 31) + ": expected count was " << expected << "\n";
        failed = true;
    }

    std::cout << captures << " captures\n";
    std::cout << en_passant << " en passants\n";
    std::cout << castles << " castles\n";
    std::cout << promotions << " promotions\n";
    std::cout << "Total nodes explored: " << total_nodes << "\n";
    std::cout << "kNodes/Sec: " << double(total_nodes) / double(test_duration) << "\n";
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout << "Usage: " << argv[0] << " <depth> <expected nodes> <fen>\n";
        return 1;
    }
    int depth = std::stoi(argv[1]);
    U64 expected_nodes = std::stoull(argv[2]);
    std::string fen = argv[3];

    std::cout << "Testing position: " << fen << "\n";
    Board b(fen);
    b.print();

    TimePoint start = get_time();
    U64 leaf_nodes = perft(b, depth);
    TimePoint test_duration = get_time() - start;

    print_results(depth, leaf_nodes, expected_nodes, test_duration);
    std::cout << "\n";
    return failed;
}