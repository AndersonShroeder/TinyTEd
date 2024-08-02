CMP = g++
TARGET = ./build/tinyted
SRC_DIR = ./src
OBJ_DIR = ./build/obj
INCLUDE_DIR = ./include

# Collect all source files
SRC = $(wildcard $(SRC_DIR)/*.cpp)
# Convert source files to object files
OBJ = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC))

CMPF = -Wall -Wextra -std=c++20 -I$(INCLUDE_DIR)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CMP) $(CMPF) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CMP) $(CMPF) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
	rm -rf $(OBJ_DIR)
