CMP = g++
target = tinyted
src = tinyted.cpp
OBJ = $(src:.cpp=.o)
CMPF = -pg -Wall -Wextra -std=c++20
all: $(target)

$(target): $(OBJ)
	$(CMP) $(CMPF) -o $@ $^

%.o: %.cpp
	$(CMP) $(CMPF) -c $< -o $@

clean:
	rm -f $(OBJ) $(target)
