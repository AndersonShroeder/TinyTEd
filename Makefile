CMP = g++
target = tinyted
src = tinyted.cpp
OBJ = $(SRC:.cpp=.0)
CMPF = -Wall -Wextra -std=c++11
LNKF =
all: $(TARGET)
$(TARGET): $(OBJ)
	$(CMP) $(LNKF) -o $@ $^

%.o: %.cpp
	$(CMP) $(CMPF) -c $< -o $@
clean:
	rm -f $(OBJ) $(TARGET)
