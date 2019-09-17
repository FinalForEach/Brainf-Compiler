
#include <iostream>
#include <map>
#include <vector>
#include <fstream>

#include "Compiler.hpp"
void compile(std::string& inputStr)
{
	
	std::map<int,int> nextBracketMap;
	std::map<int,int> prevBracketMap;
	
	//Parse brackets
	std::vector<int> openingBrackets;
	for(int i=0;i<inputStr.length();i++)
	{
		char instruction = inputStr[i];

		switch(instruction)
		{
			case '[':
			openingBrackets.push_back(i);
			break;
			case ']':
			int openValue = openingBrackets.back();
			nextBracketMap[openValue]=i;
			prevBracketMap[i]=openValue;
			openingBrackets.pop_back();
			break;
		}
	}
	
	std::string finalFileStr = "";
	
	finalFileStr+="#include <cstdlib>\n";
	finalFileStr+="#include <iostream>\n";
	finalFileStr+="int main(int argc, char **argv) \n";
	finalFileStr+="{\n";
	
	finalFileStr+="unsigned int tapeSize = 30000;\n";
	finalFileStr+="int data[tapeSize];\n";
	finalFileStr+="unsigned int dataIndex = 0;\n";
	
	//Needed for input.
	finalFileStr+="char inChar;\n";
	
	for(int i=0;i<inputStr.length();i++)
	{
		char instruction = inputStr[i];

		switch(instruction)
		{
			case '>':
			finalFileStr+="dataIndex+=1;if(dataIndex>=tapeSize)dataIndex=0;\n";
			break;
			case '<':
			finalFileStr+="dataIndex-=1;if(dataIndex<0)dataIndex=tapeSize-1;\n";
			break;
			
			case '+':
			finalFileStr+="data[dataIndex]+=1;\n";
			break;
			case '-':
			finalFileStr+="data[dataIndex]-=1;\n";
			break;
			
			case '.': 
			finalFileStr+="std::cout<<(char)data[dataIndex];\n";
			break;
			case ',':
			finalFileStr+="std::cin >> inChar;\n";
			finalFileStr+="data[dataIndex] = inChar;\n";
			break;
			
			case '[':
			finalFileStr+="while(data[dataIndex]!=0){\n";
			break;
			case ']':
			finalFileStr+="}\n";
			break;
		}
	}
	
	finalFileStr+="exit(EXIT_SUCCESS);\n";
	finalFileStr+="}\n";
	
	//std::cout<<finalFileStr;
	
	std::ofstream outputFile;
	outputFile.open("output.cpp");
	outputFile<<finalFileStr;
}
