OBJ_FOLDER = ./object-files
BIN_FOLDER = ../bin
TEST_FOLDER = ../test-programs

COMPILER = $(BIN_FOLDER)/compiler

STD_VER = c++17


SRCS = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp,$(OBJ_FOLDER)/%.o,$(SRCS))

all: COMPILER

CFLAGS = -std=$(STD_VER) -Wall -g
LFLAGS = 


COMPILER: $(OBJS)
	mkdir -p $(BIN_FOLDER); $(CXX) -o $(COMPILER) $^ $(LFLAGS)
	
	
$(OBJ_FOLDER)/%.o: %.cpp
	mkdir -p $(OBJ_FOLDER);$(CXX) -c $< -o $@ $(CFLAGS)

clean:
	rm -rf $(COMPILER)
