BIN_FOLDER = ../bin
TEST_FOLDER = ../test-programs

COMPILER = $(BIN_FOLDER)/compiler

STD_VER = c++17

all: COMPILER



COMPILER: BIN_FOLDER
	$(CXX) *.cpp -o $(COMPILER) -std=$(STD_VER)
	
BIN_FOLDER:
	mkdir -p $(BIN_FOLDER)
clean:
	rm -rf $(COMPILER)
