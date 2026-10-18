#include "uci.hpp"

void handle_uci_command(const std::string command, Board& board, Engine& engine) {
    if (command == "uci") {
        send_uci_command("id name lukechess " + version_str);
        send_uci_command("id author Luke Lyons");
        // Future options here
        send_uci_command("uciok");
    }
    else if (command == "isready")
        send_uci_command("readyok");

    else if (command.substr(0, 9) == "setoption") {
        std::string name;
        std::string value;
        sscanf(command.c_str(), " name %s", name);
        sscanf(command.c_str(), " value %s", value);
    }
    else if (command == "ucinewgame") {
        board = Board();
        engine = Engine();
    }
}

void send_uci_command(std::string command) {
    std::cout << command << "\n";
}