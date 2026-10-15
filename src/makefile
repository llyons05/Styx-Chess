BUILD_DIR = build
CXX = g++

CXXFLAGS = -g -O3 -Wall -std=c++23

_MAIN_OBJ_FILES = bitboards.o board.o movegen.o engine.o main.o
MAIN_OBJ_FILES = $(patsubst %.o,$(BUILD_DIR)/%.o,$(_MAIN_OBJ_FILES))

_TEST_OBJ_FILES = bitboards.o board.o movegen.o perft.o
TEST_OBJ_FILES = $(patsubst %.o,$(BUILD_DIR)/%.o,$(_TEST_OBJ_FILES))

HEADER_FILES = bitboards.hpp board.hpp move.hpp movegen.hpp utils.hpp

ifdef OS # Check if we are on windows
	RD = rd /s /q
	BUILD_DIR := windows-build
	DIR_GUARD = if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)
else
    RD = rm -rf
	BUILD_DIR := linux-build
    DIR_GUARD = mkdir -p $(BUILD_DIR)
endif

main: $(MAIN_OBJ_FILES)
	$(CXX) $(CXXFLAGS) $^ -o chess.exe

test: $(TEST_OBJ_FILES)
	$(CXX) $(CXXFLAGS) $^ -o test.exe

$(BUILD_DIR)/%.o: %.cpp $(HEADER_FILES)
	@$(DIR_GUARD)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	$(RD) $(BUILD_DIR)