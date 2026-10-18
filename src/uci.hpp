#include "board.hpp"
#include "engine.hpp"

#include <string>

void handle_uci_command(std::string command, Board& board, Engine& engine);
void send_uci_command(std::string command);
void send_uci_move(const Move& move);