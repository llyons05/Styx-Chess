#include "uci.hpp"

#include <ranges>
#include <string>
#include <string_view>
#include <syncstream>

UCI_Command::UCI_Command(const std::string& command, Board& board, Engine& engine, bool engine_running) 
:   command(command),
    board(board),
    engine(engine),
    engine_go(false),
    engine_stop(false),
    engine_quit(false),
    time_limit(INF_TIME),
    engine_running(engine_running) 
    {
    std::cerr << command << "\n";
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
    std::osyncstream(std::cerr) << "\t" << command << "\n";
}

void UCI_Command::id(const std::string& name, const std::string& author) {
    send_command("id name " + name);
    send_command("id author " + author);
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

void UCI_Command::info(const Engine& engine) {
    send_command("info nodes " + std::to_string(engine.nodes_traversed));
}

void UCI_Command::option(const std::string& name, const std::string& def) {
    send_command("option name " + name + " default " + def);
}

void UCI_Command::uci() {
    id("lukechess " + version_str, "Luke Lyons");
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
    // std::string name;
    // std::string value;
    // size_t name_pos = command.find("name");
    // size_t val_pos = command.find("value");
    // if (name_pos != std::string::npos)
    //     sscanf(command.c_str() + name_pos, " name %s", name);
    // if (val_pos != std::string::npos)
    //     sscanf(command.c_str() + val_pos, " value %s", value);
    // Set future options here
}

void UCI_Command::ucinewgame() {
    board = Board();
    engine = Engine();
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
        sscanf(command.c_str() + movetime, "movetime %lld", &time_limit);
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