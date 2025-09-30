CFLAGS = -O0 -Wall
SRC_FILES = bitboards.cpp main.cpp board.cpp movegen.cpp utils.cpp

main:
	g++ -o chess.exe ${CFLAGS} ${SRC_FILES}