#include "utils.hpp"

#include <chrono>
#include <bitset>
#include <iostream>

U64 get_time() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

void print_binary(U64 binary) {
    std::cout << std::bitset<64>(binary) << "\n";
}