
#include <iostream>
#include <cstdlib>
#include <map>
#include <vector>

#include "Interpreter.hpp"

//#define DEBUG 




void interpret(std::string& inputStr)
{
	unsigned int tapeSize = 30000;
	int data[tapeSize];
	unsigned int dataIndex = 0;
	
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
	
	//Interpret
	for(int i=0;i<inputStr.length();i++)
	{
		char instruction = inputStr[i];

		switch(instruction)
		{
			case '>':
			dataIndex+=1;
			if(dataIndex>=tapeSize)dataIndex=0;
			break;
			case '<':
			dataIndex-=1;
			if(dataIndex<0)dataIndex=tapeSize-1;
			break;
			
			case '+':data[dataIndex]+=1;break;
			case '-':data[dataIndex]-=1;break;
			
			case '.': std::cout<<(char)data[dataIndex]; break;
			case ',':
			char inChar;
			std::cin >> inChar;
			data[dataIndex] = inChar;
			break;
			
			//Brackets switch index to next/prev bracket,
			//which is incremented by one by forloop
			case '[':
			if(data[dataIndex]==0)i=nextBracketMap[i];
			break;
			case ']':
			if(data[dataIndex]!=0)i=prevBracketMap[i];
			break;
		}
	}
}
