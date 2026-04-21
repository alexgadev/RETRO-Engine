# ==============================
# CONFIG
# ==============================

CXX	 = g++
CC	 = gcc

CXXFLAGS = -std=c++20 -Wall -Wextra -O2 -Iinclude -Wunused-parameter
CFLAGS	 = -Wall -Wextra -Iinclude
LDFLAGS  = -lglfw -lGL -ldl -lm


SRC_DIR	 = src
BUILD_DIR = build
TARGET	 = Engine		# name of the binary 


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
