FLAGS= -pthread  -std=c++11 -O3 -w

SRC-DIR=./src
SRC-FILE=$(wildcard $(SRC-DIR)/*.cpp)

OBJ-DIR=./obj
OBJ-FILE=$(patsubst %.cpp,$(OBJ-DIR)/%.o,$(notdir $(SRC-FILE)))

BIN-DIR=./bin
BIN-FRW= $(BIN-DIR)/frw

frw: $(BIN-FRW)
$(BIN-FRW):$(OBJ-FILE)
	$(CXX) $(FLAGS) $^ -o $@
$(OBJ-FILE):$(OBJ-DIR)/%.o:$(SRC-DIR)/%.cpp
	$(CXX) $(FLAGS) -c $< -o $@

.PHONY:clean
clean:
	rm -r  $(OBJ-FILE) $(BIN-FRW)

