
#include <iostream>
#include <fstream>
#include <cstdlib>

#include "Interpreter.hpp"

#define DEBUG 
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
	
	std::string inputStr;
	inputFile >> inputStr;
	inputFile.close();
	#ifdef DEBUG
	std::cout<<"Loading file: "<<filename<<"\n";
	std::cout<<"Loaded string:"<<inputStr<<"\n";
	#endif
	interpret(inputStr);
	
	
	exit(EXIT_SUCCESS);
}



void interpret(std::string& inputStr)
{
	unsigned int data[30000];
	unsigned int *dataPointer = data;
	for(int i=0;i<inputStr.length();i++)
	{
		char instruction = inputStr[i];
		//std::cout<<"["<<i<<"]:"<<instruction<<"\n";
		
		switch(instruction)
		{
			case '>':break;
			case '<':break;
			
			case '+':break;
			case '-':break;
			
			case '.': std::cout<<*dataPointer; break;
			case ',':break;
			
			case '[':break;
			case ']':break;
		}
	}
}
