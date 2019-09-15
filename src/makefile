BIN_FOLDER = ../bin
TEST_FOLDER = ../test-programs

INTERPRETER = $(BIN_FOLDER)/interpreter

STD_VER = c++17

all: INTERPRETER


INTERPRETER:
	$(CXX) *.cpp -o $(INTERPRETER) -std=$(STD_VER)
	
	
test-helloworld: INTERPRETER
	$(INTERPRETER) $(TEST_FOLDER)/Hello\ World
	
	
clean:
	rm -rf $(INTERPRETER)
