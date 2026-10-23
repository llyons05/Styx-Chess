#pragma once

#include "board.hpp"
#include "engine.hpp"

#include <string>

class UCI_Command {
public:
    std::string command;
    Board& board;
    Engine& engine;
    bool engine_go;
    bool engine_stop;
    bool engine_quit;
    Time_Info tc_info;

    UCI_Command(const std::string& command, Board& board, Engine& engine, bool engine_running);

    static void send_command(const std::string& command);
    static void id(const std::string& name, const std::string& author);
    static void uciok();
    static void readyok();
    static void bestmove(const Move& move);
    static void info(U64 nodes, Depth depth, TimePoint start_time, Score val);
    static void option(const std::string& name, const std::string& def);

private:
    bool engine_running;
    void uci();
    void debug();
    void isready();
    void setoption();
    void ucinewgame();
    void position();
    void go();
    void stop();
    void ponderhit();
    void quit();

    void play_moves(const std::string& moves);
};