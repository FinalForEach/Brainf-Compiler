#include "CodeGen.hpp"
#include <string>
std::string getData(int cellsAway)
{
	if(cellsAway==0)
	{
		return "data[dataIndex]";
	}
	if(cellsAway>0)
	{
		std::string str =  "data[dataIndex+";
		str+=std::to_string(cellsAway);
		str+="]";
		return str;
	}else
	{
		std::string str =  "data[dataIndex-";
		str+=std::to_string(-cellsAway);
		str+="]";
		return str;		
	}
}

std::string& addLineOfCode(std::string& fileStr, const std::string& code, int indentLevel, bool addNewline=true)
{
	for(int i = 0; i< indentLevel;i++)
	{
		fileStr+="\t";
	}
	fileStr+=code;
	
	if(addNewline)fileStr+="\n";
	
	
	
	return fileStr;
}
std::string generateCode(std::vector<IRToken*>& pIRTokensVec, std::string& outputFilename)
{
	std::string code = "//Automatically generated code.\n";
	code+="#include <cstdlib>\n";
	code+="#include <iostream>\n";
	code+="#include <fstream>\n";
	code+="#include <string>\n";
	
	//Debug functions
	code+="#define FILENAME \"";code+=outputFilename;code+="\"\n";
	code+="#define DUMP_DATA std::cout<<\"\\n\"<<__LINE__<<\":\t\"; dumpData(data,tapeSize);\n";
	code+="#define DUMP_DATA_BF dumpDataToBF(data,tapeSize);std::cout<< std::string(\"DUMPED DATA AS BF AT LINE \")+std::to_string(__LINE__)+\"; EXITING.\\n\"; return 1;\n";
	
	code+="int dumpData(int data[], int tapeSize){\n";
	int curIndentLevel=1;
		addLineOfCode(code,"int maxWritten=0;",curIndentLevel);
		addLineOfCode(code,"for(int i=0;i<tapeSize;i++){",curIndentLevel);
		addLineOfCode(code,"if(data[i]!=0){maxWritten=i;}",curIndentLevel+1);
		addLineOfCode(code,"}",curIndentLevel);
		addLineOfCode(code,"std::cout<<\"Data=[\";",curIndentLevel);
		addLineOfCode(code,"for(int i=0;i<=maxWritten;i++){",curIndentLevel);
		addLineOfCode(code,"std::cout<<std::to_string(i)<<\":\"<<data[i]<<\", \";",curIndentLevel+1);
		addLineOfCode(code,"}",curIndentLevel);
		addLineOfCode(code,"std::cout<<\"]\\n\";",curIndentLevel);
	code+="}\n";
	curIndentLevel--;
	addLineOfCode(code,"void dumpDataToBF(int data[], int tapeSize){",curIndentLevel++);
	addLineOfCode(code,"int maxWritten=0;",curIndentLevel);
	addLineOfCode(code,"for(int i=0;i<tapeSize;i++){if(data[i]!=0){maxWritten=i;}}",curIndentLevel);
	addLineOfCode(code,"std::string bfOutStr = \"\";",curIndentLevel);
	addLineOfCode(code,"for(int i=0;i<=maxWritten;i++){",curIndentLevel++);
	addLineOfCode(code,"int val = data[i];",curIndentLevel);
	addLineOfCode(code,"if(val>0){for(int v=0;v<val;v++){bfOutStr+=\"+\";}}",curIndentLevel);
	addLineOfCode(code,"if(val<0){for(int v=0;v<-val;v++){bfOutStr+=\"-\";}}",curIndentLevel);
	addLineOfCode(code,"bfOutStr+=\">\";",curIndentLevel--);
	addLineOfCode(code,"}",curIndentLevel);
	addLineOfCode(code,"for(int i=0;i<=maxWritten;i++){bfOutStr+=\"<\";}//Rewind tape for portability",curIndentLevel);
	addLineOfCode(code,"std::ofstream outputFile(FILENAME \"-data-dump.b\", std::ofstream::out);",curIndentLevel);
	addLineOfCode(code,"outputFile<<bfOutStr;outputFile.close();",curIndentLevel--);
	addLineOfCode(code,"}",curIndentLevel);
	
	//Main function
	code+="int main(int argc, char **argv) \n";
	code+="{\n";
	curIndentLevel++;
	addLineOfCode(code,"//Setup data cells",curIndentLevel);
	addLineOfCode(code,"const unsigned int tapeSize = 60000;",curIndentLevel);
	addLineOfCode(code,"int data[tapeSize] = {};",curIndentLevel);
	addLineOfCode(code,"register unsigned int dataIndex = 30000;",curIndentLevel);
	addLineOfCode(code,"//Start program",curIndentLevel);
	for(unsigned int i=0;i<pIRTokensVec.size();i++)
	{
		IRToken *irToken= pIRTokensVec[i];
		curIndentLevel+=irToken->getPreIndentModifier();
		
		bool addTokenComment=true;
		
		addLineOfCode(code,irToken->generateCode(),curIndentLevel,!addTokenComment);
		if(addTokenComment)
		{
			addLineOfCode(code,"// IRToken: ",0,false);
			addLineOfCode(code,irToken->getName(),0,false);
			addLineOfCode(code,"#",0,false);
			addLineOfCode(code,std::to_string(irToken->getIRTokenID()),0,true);
			
		}
		curIndentLevel+=irToken->getPostIndentModifier();
	}
	
	code+="}\n";
	
	return code;
}

std::string IRTokenMultiAdd::generateCode() const
{
	if(intVal!=0)
	{
		std::string code = getData(cellsAway);
	
		if(intVal>0)
		{
			code+="+="+std::to_string(intVal);
		}else
		{
			code+="-="+std::to_string(-intVal);
		}
		code+=";";
		return code;
	}else
	{
		std::string code = "//Would have added "+std::to_string(intVal)
			+" to "+getData(cellsAway);
		return code;
	}
	
}
std::string IRTokenMultiShift::generateCode() const
{
	std::string code = "";
	if(numShifts>0)
	{
		code+="dataIndex+="+std::to_string(numShifts)+";";
	}else
	{
		code+="dataIndex-="+std::to_string(-numShifts)+";";
	}
	return code;
}
std::string IRTokenClear::generateCode() const
{
	std::string code = getData(cellsAway);
	code+="=";
	code+=std::to_string(setVal);
	code+=";";
	return code;	
}
std::string IRTokenLoopOpen::generateCode() const
{
	std::string code = "while(";
	code+=getData(cellsAway);
	code+="!=0){";
	return code;	
}
std::string IRTokenLoopClose::generateCode() const
{
	return "}";	
}
std::string IRTokenIfOpen::generateCode() const
{
	std::string code = "if(";
	code+=getData(cellsAway);
	code+="!=0){";
	return code;
}
std::string IRTokenIfClose::generateCode() const
{
	if(doesClear)
	{
		std::string code = getData(cellsAway);
		code+="}";
		return code;
	}else
	{
		return "}";	
	}
}
std::string IRTokenInput::generateCode() const
{
	return "std::cin >> "+getData(cellsAway)+";";	
}
std::string IRTokenPrintChar::generateCode() const
{
	//std::string code = "std::cout<<";
	std::string code = "putchar(";
	if(hasKnownCharValue())
	{
		int c = knownCharValue.value();
		if(c >= 32 && c <= 126)
		{
			code+="'";
			switch(c)
			{
				case '\'':
				case '\\': //Require escape chars
				code+='\\';
				code+=c;
				break;
				default:
				code+=c;
				break;
			}
			code+="');";
			return code;
		}else
		{
			//code+="(char)";
			code+=std::to_string((int)c);
			code+=");";
			return code;
		}
	}
	//code+="(char)";
	code+=getData(cellsAway);
	code+=");";
	return code;	
}

std::string IRTokenMultiply::generateCode() const
{
	std::string code =getData(cellsAway)+"+="+getData(factorACellsAway)+" * "+std::to_string(factor)+";";
	return code;
}

std::string IRTokenPrintStr::generateCode() const
{
	std::string code = "std::cout<<\"";
	for(char c : str)
	{
		if(c >= 32 && c <= 126)
		{
			switch(c)
			{
				case '\'':
				case '\"':
				case '\\': //Require escape chars
				code+='\\';
				code+=c;
				break;
				default:
				code+=c;
				break;
			}
		}else
		{
			switch(c)
			{
				case '\n':
				code+="\n";break;
				case '\r':
				code+="\r";break;
			}
		}
	}	
	code+="\";";
	return code;
}
