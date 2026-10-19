BUILD_DIR = build
OUTPUT_DIR = ../engines
CXX = g++

VERSION ?= 0

CXXFLAGS = -g -O3 -Wall -std=c++23 -DVERSION_STR=$(VERSION)

_MAIN_OBJ_FILES = bitboards.o board.o movegen.o engine.o uci.o main.o
MAIN_OBJ_FILES = $(patsubst %.o,$(BUILD_DIR)/%.o,$(_MAIN_OBJ_FILES))

_TEST_OBJ_FILES = bitboards.o board.o movegen.o perft.o
TEST_OBJ_FILES = $(patsubst %.o,$(BUILD_DIR)/%.o,$(_TEST_OBJ_FILES))

HEADER_FILES = bitboards.hpp board.hpp engine.hpp move.hpp movegen.hpp utils.hpp uci.hpp

ifeq ($(OS),Windows_NT)
	TARGET = $(OUTPUT_DIR)/LukeChess_v$(VERSION)-windows.exe
	BUILD_DIR := windows-build
    ifeq ($(MSYSTEM),)
        RD = rd /s /q
        BUILD_GUARD = if not exist "$(BUILD_DIR)" mkdir "$(BUILD_DIR)"
        OUTPUT_GUARD = if not exist "$(OUTPUT_DIR)" mkdir "$(OUTPUT_DIR)"
    else
        RD = rm -rf
        BUILD_GUARD = mkdir -p "$(BUILD_DIR)"
        OUTPUT_GUARD = mkdir -p "$(OUTPUT_DIR)"
    endif
else
	TARGET = $(OUTPUT_DIR)/LukeChess_v$(VERSION)-linux.exe
    RD = rm -rf
    BUILD_DIR := linux-build
    BUILD_GUARD = mkdir -p "$(BUILD_DIR)"
    OUTPUT_GUARD = mkdir -p "$(OUTPUT_DIR)"
endif

main: $(MAIN_OBJ_FILES)
	@$(OUTPUT_GUARD)
	$(CXX) $(CXXFLAGS) $^ -o $(TARGET)

test: $(TEST_OBJ_FILES)
	$(CXX) $(CXXFLAGS) $^ -o test.exe

$(BUILD_DIR)/%.o: %.cpp $(HEADER_FILES)
	@$(BUILD_GUARD)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	$(RD) $(BUILD_DIR)