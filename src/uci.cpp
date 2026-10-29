#include "uci.hpp"

#include <math.h>
#include <ranges>
#include <string>
#include <string_view>
#include <syncstream>

UCI_Options uci_options;

UCI_Command::UCI_Command(const std::string& command, Board& board, Engine& engine, bool engine_running) 
:   command(command),
    board(board),
    engine(engine),
    engine_go(false),
    engine_stop(false),
    engine_quit(false),
    tc_info(),
    engine_running(engine_running) 
    {
    if (command == "uci")
        uci();
    else if (command.starts_with("debug"))
        debug();
    else if (command == "isready")
        isready();
    else if (command.starts_with("setoption"))
        setoption();
    else if (command == "ucinewgame" && !engine_running)
        ucinewgame();
    else if (command.starts_with("position") && !engine_running)
        position();
    else if (command.starts_with("go") && !engine_running)
        go();
    else if (command == "stop")
        stop();
    else if (command == "ponderhit")
        ponderhit();
    else if (command == "quit")
        quit();
}

void UCI_Command::send_command(const std::string& command) {
    std::osyncstream(std::cout) << command << "\n";
    std::osyncstream(std::cout).flush();
}

void UCI_Command::id(const std::string& name, const std::string& author) {
    send_command("id name " + name);
    send_command("id author " + author);
    option("Hash", "type spin default 1 min 1 max 128");
}

void UCI_Command::uciok() {
    send_command("uciok");
}

void UCI_Command::readyok() {
    send_command("readyok");
}

void UCI_Command::bestmove(const Move& move) {
    send_command("bestmove " + move.long_alg());
}

void UCI_Command::info(U64 nodes, Depth depth, TimePoint start_time, Score score, U64 hash_full) {
    std::string info_str = "info nodes " + std::to_string(nodes) + " depth " + std::to_string(depth) + " score ";
    if (abs(score) >= MIN_WINNING_SCORE) {
        if (score < 0)
            info_str += "mate " + std::to_string(int(std::floor(double(-MAX_SCORE - score)/2)));
        else
            info_str += "mate " + std::to_string(int(std::ceil(double(MAX_SCORE - score)/2)));
    }
    else
        info_str += "cp " + std::to_string(score);
    
    TimePoint elapsed = get_time() - start_time;
    if (elapsed)
        info_str += " nps " + std::to_string(int64_t(1000 * double(nodes)/double(elapsed)));
    
    info_str += " hashfull " + std::to_string(hash_full);
    send_command(info_str);
}

void UCI_Command::option(const std::string& name, const std::string& opts) {
    send_command("option name " + name + " " + opts);
}

void UCI_Command::uci() {
    id("Styx " + version_str, "Luke Lyons");
    // Future options here
    UCI_Command::uciok();
}

void UCI_Command::debug() {
    // There will probably be something here someday
}

void UCI_Command::isready() {
    UCI_Command::readyok();
}

void UCI_Command::setoption() {
    std::string name, value;
    size_t name_pos = command.find("name");
    size_t val_pos = command.find("value");
    if (name_pos != std::string::npos)
        std::stringstream(command.substr(name_pos+5)) >> name;
    if (val_pos != std::string::npos)
        std::stringstream(command.substr(val_pos+5)) >> value;
    
    if (name.empty()) return;

    if ((name == "Hash") && !engine_running) {
        uci_options.hash_size = U64(std::stoull(value)* 1000000);
        engine = Engine(uci_options.hash_size);
    }
}

void UCI_Command::ucinewgame() {
    board = Board();
    engine.new_game();
}

void UCI_Command::position() {
    if (command.substr(0, 12) == "position fen")
        board = Board(command.substr(13));
    else
        board = Board();
    
    size_t moves = command.find("moves");
    if (moves != std::string::npos)
        play_moves(command.substr(moves+6));
}

void UCI_Command::go() {
    size_t movetime = command.find("movetime");
    if (movetime != std::string::npos) {
        sscanf(command.c_str() + movetime, "movetime %lld", &tc_info.movetime);
    }
    size_t wtime = command.find("wtime");
    if (wtime != std::string::npos) {
        sscanf(command.c_str() + wtime, "wtime %lld", &tc_info.remaining[WHITE]);
    }
    size_t btime = command.find("btime");
    if (btime != std::string::npos) {
        sscanf(command.c_str() + btime, "btime %lld", &tc_info.remaining[BLACK]);
    }
    size_t winc = command.find("winc");
    if (winc != std::string::npos) {
        sscanf(command.c_str() + winc, "winc %lld", &tc_info.inc[WHITE]);
    }
    size_t binc = command.find("binc");
    if (binc != std::string::npos) {
        sscanf(command.c_str() + binc, "binc %lld", &tc_info.inc[BLACK]);
    }
    size_t moves_to_go = command.find("movestogo");
    if (moves_to_go != std::string::npos) {
        sscanf(command.c_str() + moves_to_go, "movestogo %d", &tc_info.moves_to_go);
    }
    engine.external_cancel.store(false, std::memory_order::relaxed);
    engine_go = true;
}

void UCI_Command::stop() {
    engine.external_cancel.store(true, std::memory_order::relaxed);
    engine_stop = true;
}

void UCI_Command::ponderhit() {
    go();
}

void UCI_Command::quit() {
    engine.external_cancel.store(true, std::memory_order::relaxed);
    engine_quit = true;
}

void UCI_Command::play_moves(const std::string& moves) {
    auto split_view = moves | std::views::split(' ');

    for (auto&& s : split_view) {
        // Convert the subrange to a string_view for easy printing/usage
        std::string_view move_str(s.begin(), s.end());
        if (move_str.empty())
            continue;
        
        Move move_to_make;
        Move_Generator mg(board);
        for (const Move& m : mg.movelist) {
            if (m.long_alg() == move_str) {
                move_to_make = m;
                break;
            }
        }
        if (move_to_make.is_valid())
            board.move(move_to_make);
        else
            break;
    }
}