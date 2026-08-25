# ==============================
# CONFIG
# ==============================

CXX	 = g++
CC	 = gcc

CXXFLAGS = -std=c++20 -Wall -Wextra -O2 -Iinclude
CFLAGS	 = -Wall -Wextra -Iinclude

SRC_DIR	 = src
BUILD_DIR = build
# name of the binary
TARGET	 = Engine

UNAME_S := $(shell uname -s 2>/dev/null)

ifeq ($(OS),Windows_NT)
	IS_WINDOWS := 1
else ifneq (,$(findstring MINGW,$(UNAME_S)))
	IS_WINDOWS := 1
else ifneq (,$(findstring MSYS,$(UNAME_S)))
	IS_WINDOWS := 1
else
	IS_WINDOWS := 0
endif

ifeq ($(IS_WINDOWS),1)
	LDFLAGS = -lglfw3 -lopengl32 -lgdi32 -lm -lfreetype
	TARGET  := $(TARGET).exe
else
	LDFLAGS = -lglfw -lGL -ldl -lm -lfreetype
endif


SRC  = $(shell find $(SRC_DIR) -name "*.cpp") $(shell find $(SRC_DIR) -name "*.c")
OBJ  = $(SRC:src/%.cpp=$(BUILD_DIR)/%.o)
OBJ  := $(OBJ:src/%.c=$(BUILD_DIR)/%.o)


all: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) $(LDFLAGS) -o $(TARGET)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) -c $< $(CXXFLAGS) -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) -c $< $(CFLAGS) -o $@

clean: 
	rm -rf $(BUILD_DIR) $(TARGET)

run: $(TARGET)
	./$(TARGET)

re: clean all
