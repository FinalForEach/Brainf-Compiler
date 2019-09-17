
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
	int indentLevel=1;
	
	addLineOfCode(finalFileStr,"unsigned int tapeSize = 30000;",indentLevel);
	addLineOfCode(finalFileStr,"int data[tapeSize];",indentLevel);
	addLineOfCode(finalFileStr,"unsigned int dataIndex = 0;",indentLevel);
	
	
	//Needed for input.
	addLineOfCode(finalFileStr,"char inChar;",indentLevel);
	
	int curVarValue=0;
	
	for(int i=0;i<inputStr.length();i++)
	{
		char instruction = inputStr[i];

		switch(instruction)
		{
			case '+':
			curVarValue+=1;
			break;
			case '-':
			curVarValue-=1;
			break;
			default:
			if(curVarValue!=0)
			{
				addLineOfCode(finalFileStr,"data[dataIndex]+=",indentLevel,false);
				addLineOfCode(finalFileStr,std::to_string(curVarValue),0,false);
				addLineOfCode(finalFileStr,";",0,true);
				curVarValue=0;
			}
			break;
		}
		switch(instruction)
		{
			case '>':
			addLineOfCode(finalFileStr,"dataIndex+=1;if(dataIndex>=tapeSize)dataIndex=0;",indentLevel);
			break;
			case '<':
			addLineOfCode(finalFileStr,"dataIndex-=1;if(dataIndex<0)dataIndex=tapeSize-1;",indentLevel);
			break;
			
			
			case '.': 
			addLineOfCode(finalFileStr,"std::cout<<(char)data[dataIndex];",indentLevel);
			break;
			case ',':
			addLineOfCode(finalFileStr,"std::cin >> inChar;",indentLevel);
			addLineOfCode(finalFileStr,"data[dataIndex] = inChar;",indentLevel);
			break;
			
			case '[':
			addLineOfCode(finalFileStr,"while(data[dataIndex]!=0){",indentLevel);
			indentLevel+=1;
			break;
			case ']':
			indentLevel-=1;
			addLineOfCode(finalFileStr,"}",indentLevel);
			break;
		}
	}
	
	addLineOfCode(finalFileStr,"exit(EXIT_SUCCESS);",indentLevel);
	finalFileStr+="}\n";
	
	//std::cout<<finalFileStr;
	
	std::ofstream outputFile;
	outputFile.open("output.cpp");
	outputFile<<finalFileStr;
	outputFile.close();
}
std::string& addLineOfCode(std::string& fileStr, const std::string& code, int indentLevel, bool addNewline)
{
	for(int i = 0; i< indentLevel;i++)
	{
		fileStr+="\t";
	}
	fileStr+=code;
	
	if(addNewline)fileStr+="\n";
	
	return fileStr;
}
