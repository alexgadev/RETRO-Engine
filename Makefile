# ==============================
# CONFIG
# ==============================

CXX	 = g++
CC	 = gcc

CXXFLAGS = -std=c++20 -Wall -Wextra -O2 -Iinclude
CFLAGS	 = -Wall -Wextra -Iinclude

GLFW_FLAGS = $(shell pkg-config --cflags --libs glfw3)

LIBS = -lGL -ldl -lpthread

TARGET	 = RETRO-Engine		# name of the binary 
SRC_DIR	 = source		# origin source files' folder
OBJ_DIR	 = build		# origin build files' folder

# ==============================
# SOURCES
# ==============================

CPP_SOURCE  := $(shell find $(SRC_DIR) -name "*.cpp")
C_SOURCE    :=  $(shell find $(SRC_DIR) -name "*.c")

CPP_OBJECTS :=  $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(CPP_SOURCES))
C_OBJECTS   :=  $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(C_SOURCES))

OBJECTS = $(CPP_OBJECTS) $(C_OBJECTS)

# ==============================
# RULES
# ==============================

all: $(TARGET)

$(TARGET): $(OBJECTS)
    $(CXX) $(OBJECTS) -o $(TARGET) $(GLFW_FLAGS) $(LIBS)

# Compilar .cpp
$(OBJ_DIR)/%.o: $(SRC_DIR=/%.cpp
    @mkdir -p $(dir $@)
    $(CXX) $(CXXFLAGS) -c $< -o $@

# Compilar .c
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
    @mkdir -p $(dir $@)
    $(CC) $(CFLAGS) -c $< -o $@

clean: 
    rm -rf $(OBJ_DIR) $(TARGET)
	
re: clean all
