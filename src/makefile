BIN_FOLDER = ../bin
TEST_FOLDER = ../test-programs

INTERPRETER = $(BIN_FOLDER)/interpreter

all: INTERPRETER


INTERPRETER:
	$(CXX) *.cpp -o $(INTERPRETER)
	
	
test-helloworld: INTERPRETER
	$(INTERPRETER) $(TEST_FOLDER)/Hello\ World
	
	
clean:
	rm -rf $(INTERPRETER)
