
#include <iostream>
#include <map>
#include <vector>
#include <fstream>

#include "Compiler.hpp"
void compile(std::string& inputStr)
{
	

	bool inputRequired=false;
	for(int i=0;i<inputStr.length();i++)
	{
		char instruction = inputStr[i];

		switch(instruction)
		{
			case ',':
			inputRequired=true;
			break;
		}
	}
	
	std::string finalFileStr = "";
	
	finalFileStr+="//Automatically generated code.\n";
	finalFileStr+="#include <cstdlib>\n";
	finalFileStr+="#include <iostream>\n";
	finalFileStr+="int main(int argc, char **argv) \n";
	finalFileStr+="{\n";
	int indentLevel=1;
	
	addLineOfCode(finalFileStr,"//Setup data cells\n",indentLevel);
	addLineOfCode(finalFileStr,"const unsigned int tapeSize = 30000;",indentLevel);
	addLineOfCode(finalFileStr,"int data[tapeSize];",indentLevel);
	addLineOfCode(finalFileStr,"unsigned int dataIndex = 0;",indentLevel);
	
	
	if(inputRequired){
		//Needed for input.
		addLineOfCode(finalFileStr,"char inChar;",indentLevel);
	}
	
	addLineOfCode(finalFileStr,"\n",indentLevel);
	addLineOfCode(finalFileStr,"//Start program.\n",indentLevel);
	
	int curVarValue=0;
	int curShiftValue=0;
	
	for(int i=0;i<inputStr.length();i++)
	{
		char instruction = inputStr[i];

		//Addition / Subtraction
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
				if(curVarValue>0)
				{
					addLineOfCode(finalFileStr,"data[dataIndex]+=",indentLevel,false);
					addLineOfCode(finalFileStr,std::to_string(curVarValue),0,false);
				}else
				{
					addLineOfCode(finalFileStr,"data[dataIndex]-=",indentLevel,false);
					addLineOfCode(finalFileStr,std::to_string(-curVarValue),0,false);
				}
				addLineOfCode(finalFileStr,";",0,true);
				curVarValue=0;
			}
			break;
		}
		//Shift Left / Right
		switch(instruction)
		{
			case '>':
			curShiftValue+=1;
			break;
			case '<':
			curShiftValue-=1;
			break;
			default:
			if(curShiftValue!=0)
			{
				//The shift
				if(curShiftValue>0)
				{
					addLineOfCode(finalFileStr,"dataIndex+=",indentLevel,false);
					addLineOfCode(finalFileStr,std::to_string(curShiftValue),0,false);
				}else
				{
					addLineOfCode(finalFileStr,"dataIndex-=",indentLevel,false);
					addLineOfCode(finalFileStr,std::to_string(-curShiftValue),0,false);
				}
				addLineOfCode(finalFileStr,";",0,true);
				
				//Bounds wrapping, may change later if I want unlimited tape
				if(curShiftValue>0)
				{
					addLineOfCode(finalFileStr,"if(dataIndex>=tapeSize)dataIndex%=tapeSize;",indentLevel,true);
				}else
				{
					addLineOfCode(finalFileStr,"if(dataIndex<0)dataIndex=tapeSize+dataIndex;",indentLevel,true);
				}
				
				
				
				curShiftValue=0;
			}
			break;
		}
		//Rest of instructions
		switch(instruction)
		{
				
			
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
	
	addLineOfCode(finalFileStr,"\n\n",indentLevel);
	addLineOfCode(finalFileStr,"exit(EXIT_SUCCESS);//End program",indentLevel);
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
