CMP = g++
TARGET = ./build/tinyted
TEST_TARGET = ./build/tcp_server_test
CLIENT_TEST_TARGET = ./build/tcp_client_test
SRC_DIR = ./src
OBJ_DIR = ./build/obj
INCLUDE_DIR = ./include
TEST_SRC = ./tests/tcp_server.cpp  # Adjust this path as needed
CLIENT_TEST_SRC = ./tests/tcp_client.cpp  # Adjust this path as needed

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
	rm -f $(OBJ) $(TARGET) $(TEST_TARGET) $(CLIENT_TEST_TARGET)
	rm -rf $(OBJ_DIR)

test: $(TEST_TARGET) $(CLIENT_TEST_TARGET)

$(TEST_TARGET): $(TEST_SRC)
	$(CMP) $(CMPF) -o $@ $<

$(CLIENT_TEST_TARGET): $(CLIENT_TEST_SRC)
	$(CMP) $(CMPF) -o $@ $<
