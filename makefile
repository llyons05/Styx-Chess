CXXFLAGS = -O0 -Wall -std=c++23
SRC_FILES = bitboards.cpp main.cpp board.cpp movegen.cpp

main:
	g++ -o chess.exe ${CXXFLAGS} ${SRC_FILES}