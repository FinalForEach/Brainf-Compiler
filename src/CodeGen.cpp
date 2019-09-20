#include "CodeGen.hpp"
#include "Compiler.hpp"

std::string generateCode(std::vector<IRToken*>& pIRTokensVec)
{
	std::string code = "//Automatically generated code.\n";
	code+="#include <cstdlib>\n";
	code+="#include <iostream>\n";
	code+="int main(int argc, char **argv) \n";
	code+="{\n";
	
	int curIndentLevel=1;
	addLineOfCode(code,"//Setup data cells\n",curIndentLevel);
	addLineOfCode(code,"const unsigned int tapeSize = 30000;",curIndentLevel);
	addLineOfCode(code,"int data[tapeSize];",curIndentLevel);
	addLineOfCode(code,"unsigned int dataIndex = 0;",curIndentLevel);
	for(int i=0;i<pIRTokensVec.size();i++)
	{
		IRToken *irToken= pIRTokensVec[i];
		curIndentLevel+=irToken->getPreIndentModifier();
		addLineOfCode(code,irToken->generateCode(),curIndentLevel);
		curIndentLevel+=irToken->getPostIndentModifier();
	}
	
	code+="}\n";
	
	return code;
}

std::string IRTokenMultiAdd::generateCode() const
{
	if(intVal!=0)
	{
		std::string code = "data[";
		if(cellsAway!=0)
		{
			if(cellsAway>0)
			{
				code += "dataIndex+";
				code+=std::to_string(cellsAway);
				code+="]+=";
			}else
			{
				code += "dataIndex-";
				code+=std::to_string(-cellsAway);
				code+="]+=";
			}
		}else
		{
			code += "dataIndex]+=";
		}
		code+=std::to_string(intVal);
		code+=";";
		return code;
	}else
	{
		return "";
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
	std::string code = "data[dataIndex]=0;";
	return code;	
}
std::string IRTokenLoopOpen::generateCode() const
{
	return "while(data[dataIndex]!=0){";	
}
std::string IRTokenLoopClose::generateCode() const
{
	return "}";	
}
std::string IRTokenInput::generateCode() const
{
	return "std::cin >> data[dataIndex];";	
}
std::string IRTokenPrintChar::generateCode() const
{
	if(hasKnownCharValue())
	{
		char c = knownCharValue.value();
		std::cout<<"Knowing value: "<<c<<"\n";
		if(c >= 32 && c <= 126)
		{
			std::string code = "std::cout<<'";
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
			code+="'";
			return code;
		}
	}
	return "std::cout<<(char)data[dataIndex];";	
}

std::string IRTokenMultiply::generateCode() const
{
	std::string code = "data[dataIndex]=data[dataIndex+";
	code+=std::to_string(cellsAway);
	code+="] * ";
	code+=std::to_string(factor);
	code+=";";
	code+= "data[dataIndex]=0;";//Clears the cell
	return code;	
}
