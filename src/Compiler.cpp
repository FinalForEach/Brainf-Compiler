
#include <iostream>
#include <map>
#include <vector>

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
	finalFileStr+="int main(int argc, char **argv) \n";
	finalFileStr+="{\n";
	
	finalFileStr+="unsigned int tapeSize = 30000;\n";
	finalFileStr+="int data[tapeSize];\n";
	finalFileStr+="unsigned int dataIndex = 0;\n";
	
	
	for(int i=0;i<inputStr.length();i++)
	{
		char instruction = inputStr[i];

		switch(instruction)
		{
			
			case '+':finalFileStr+="data[dataIndex]+=1;\n";break;
			case '-':finalFileStr+="data[dataIndex]-=1;\n";break;
		}
	}
	
	finalFileStr+="\t";finalFileStr+="exit(EXIT_SUCCESS)\n";
	finalFileStr+="}\n";
	
	std::cout<<finalFileStr;
}
