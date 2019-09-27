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
std::string generateCode(std::vector<IRToken*>& pIRTokensVec)
{
	std::string code = "//Automatically generated code.\n";
	code+="#include <cstdlib>\n";
	code+="#include <iostream>\n";
	code+="int main(int argc, char **argv) \n";
	code+="{\n";
	
	int curIndentLevel=1;
	addLineOfCode(code,"//Setup data cells",curIndentLevel);
	addLineOfCode(code,"const unsigned int tapeSize = 30000;",curIndentLevel);
	addLineOfCode(code,"int data[tapeSize] = {};",curIndentLevel);
	addLineOfCode(code,"unsigned int dataIndex = 0;",curIndentLevel);
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
			addLineOfCode(code,irToken->getName(),0,true);
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
			code+="+=";			
			code+=std::to_string(intVal);
		}else
		{
			code+="-=";
			code+=std::to_string(-intVal);
		}
		code+=";";
		return code;
	}else
	{
		std::string code = "//Would have added ";
		code+=std::to_string(intVal);
		code+=" to ";
		code+=getData(cellsAway);
		return code;
	}
	
}
std::string IRTokenMultiShift::generateCode() const
{
	std::string code = "";
	if(numShifts>0)
	{
		code+="dataIndex+=";
		code+=std::to_string(numShifts);
		code+=";";
		code+="if(dataIndex>=tapeSize)dataIndex%=tapeSize;";
	}else
	{
		code+="dataIndex-=";
		code+=std::to_string(-numShifts);
		code+=";";
		code+="if(dataIndex<0)dataIndex=tapeSize+dataIndex;";
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
	return "std::cin >> data[dataIndex];";	
}
std::string IRTokenPrintChar::generateCode() const
{
	std::string code = "std::cout<<";
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
			code+="';";
			return code;
		}else
		{
			code+="(char)";
			code+=std::to_string((int)c);
			code+=";";
			return code;
		}
	}
	code+="(char)";
	code+=getData(cellsAway);
	code+=";";
	return code;	
}

std::string IRTokenMultiply::generateCode() const
{
	std::string code =getData(cellsAway);
	code+="+=";
	code+=getData(factorACellsAway);
	code+=" * ";
	code+=std::to_string(factor);
	code+=";";
	if(doClear){
		code+= "data[dataIndex]=0;";//Clears the cell
	}
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
