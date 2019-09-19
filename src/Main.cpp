#include <iostream>
#include <fstream>
#include <cstdlib>
#include "Utils.hpp"
#include "Compiler.hpp"
#include "Tokenizer.hpp"
#include "IR.hpp"
#include "CodeGen.hpp"
int main(int argc, char **argv)
{
	if(argc<=1)
	{
		std::cerr<<"Need to pass in a file as an argument\n";
		
		exit(EXIT_FAILURE);
	}
	std::string filename = argv[1];
	
	
	std::ifstream inputFile(filename);
	if(inputFile.fail())
	{
		std::cerr<<"Could not find file: "<<filename<<"\n";
		inputFile.close();
		exit(EXIT_FAILURE);
	}
		
	std::string inputStr = loadStrFromFile(inputFile);
	
	inputFile.close();
	#ifdef DEBUG
	std::cout<<"Loading file: "<<filename<<"\n";
	std::cout<<"Loaded string:"<<inputStr<<"\n";
	#endif
	//compile(inputStr);
	
	std::vector<Token*> pTokensVec;
	
	std::cout<<"Tokenizing:\n";
	
	tokenizeString(inputStr, pTokensVec);
	printTokens(pTokensVec);
	
	std::cout<<"\n";
	std::cout<<"\n";
	std::cout<<"Converting to IRTokens:\n";
	
	std::vector<IRToken*> pIRTokensVec;
	convertTokensToIR(pTokensVec, pIRTokensVec);
	printIRTokens(pIRTokensVec);
	
	std::cout<<"\n";
	std::cout<<"\n";
	std::cout<<"Generating code:\n";
	
	std::string code = generateCode(pIRTokensVec);
	std::cout<<code;
	
	std::ofstream outputFile;
	outputFile.open("../output.cpp");
	outputFile<<code;
	outputFile.close();
	
	exit(EXIT_SUCCESS);
}
