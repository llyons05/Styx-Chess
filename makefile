CXXFLAGS = -O3 -Wall -std=c++23
SRC_FILES = bitboards.cpp board.cpp movegen.cpp

main:
	g++ -o chess.exe ${CXXFLAGS} ${SRC_FILES} main.cpp

test:
	g++ -o test.exe ${CXXFLAGS} ${SRC_FILES} perft.cpp