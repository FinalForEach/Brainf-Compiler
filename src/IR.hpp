#pragma once
#include <vector>
#include <iostream>
#include <string>
#include <optional>
#include "Tokenizer.hpp"




class IRToken
{
	public:
	IRToken(){}
	~IRToken(){}
	virtual std::string getName() const
	{
		return "UNKNOWN_IRTOKEN";
	}
	virtual std::string generateCode() const{}
	virtual int getPreIndentModifier() const
	{
		return 0;
	}
	virtual int getPostIndentModifier() const
	{
		return 0;
	}
};
class IRTokenComment : public IRToken
{
	std::string commentStr;
	public:
	IRTokenComment(std::string str) : commentStr(str), IRToken()
	{
		
	}
	std::string generateCode() const override
	{
		std::string code = "/*";
		code+=commentStr;
		code+="*/";
		return code;
	}
	std::string getName() const override
	{
		return "IRTokenComment";
	}
};
class IRTokenNoOp : public IRToken //Used for replacing IRTokens, while preserving iteration
{
	IRToken *overridenIRToken;
	public:
	IRTokenNoOp() : overridenIRToken(nullptr), IRToken()
	{}
	IRTokenNoOp(IRToken *deletedIr) : overridenIRToken(deletedIr), IRToken()
	{}
	std::string getName() const override
	{
		return "IRTokenNoOp";
	}
	std::string generateCode() const override
	{
		if(overridenIRToken!=nullptr)
		{
			std::string code = "//Overwritten ";
			code+=overridenIRToken->getName();
			return code;
		}
		return "";
	}
};
class IRTokenMultiAdd : public IRToken
{
	public:
	int intVal;
	int cellsAway;
	IRTokenMultiAdd(int i): intVal(i), cellsAway(0), IRToken()
	{
	}
	IRTokenMultiAdd(int i, int c): intVal(i), cellsAway(c), IRToken()
	{
	}
	std::string getName() const override
	{
		return "IRTokenMultiAdd";
	}
	std::string generateCode() const override;
};
class IRTokenMultiShift : public IRToken
{
	public:
	int numShifts;
	IRTokenMultiShift(int i)
	: numShifts(i),
	IRToken()
	{
	}
	std::string getName() const override
	{
		return "IRTokenMultiShift";
	}
	std::string generateCode() const override;
};
class IRTokenClear : public IRToken
{
	public:
	int setVal;
	int cellsAway;
	IRTokenClear(): setVal(0), cellsAway(0),IRToken()
	{
	}
	IRTokenClear(int v, int c): setVal(v), cellsAway(c),IRToken()
	{
	}
	std::string getName() const override
	{
		return "IRTokenClear";
	}
	std::string generateCode() const override;
};
class IRTokenLoopOpen : public IRToken
{
	public:
	IRTokenLoopOpen()
	: IRToken()
	{
	}
	std::string getName() const override
	{
		return "IRTokenLoopOpen";
	}
	std::string generateCode() const override;
	int getPostIndentModifier() const override
	{
		return 1;
	}
};
class IRTokenLoopClose : public IRToken
{
	public:
	IRTokenLoopClose()
	: IRToken()
	{
	}
	std::string getName() const override
	{
		return "IRTokenLoopClose";
	}
	std::string generateCode() const override;
	int getPreIndentModifier() const override
	{
		return -1;
	}
};

class IRTokenIfOpen : public IRToken
{
	public:
	IRTokenIfOpen()
	: IRToken()
	{
	}
	std::string getName() const override
	{
		return "IRTokenIfOpen";
	}
	std::string generateCode() const override;
	int getPostIndentModifier() const override
	{
		return 1;
	}
};
class IRTokenIfClose : public IRToken
{
	public:
	bool doClear;
	IRTokenIfClose(bool _doClear)
	: doClear(_doClear), IRToken()
	{
	}
	std::string getName() const override
	{
		return "IRTokenIfClose";
	}
	std::string generateCode() const override;
	int getPreIndentModifier() const override
	{
		return -1;
	}
};

class IRTokenInput : public IRToken
{
	public:
	IRTokenInput()
	: IRToken()
	{
	}
	std::string getName() const override
	{
		return "IRTokenInput";
	}
	std::string generateCode() const override;
};
class IRTokenPrintChar : public IRToken
{
	public:
	std::optional<int> knownCharValue;
	IRTokenPrintChar(): IRToken()
	{
	}
	IRTokenPrintChar(int c): knownCharValue(c), IRToken()
	{
	}
	std::string getName() const override
	{
		return "IRTokenPrintChar";
	}
	std::string generateCode() const override;
	
	
	bool hasKnownCharValue() const
	{
		return knownCharValue.has_value();
	}
	bool isLiteralChar() const
	{
		if(!hasKnownCharValue())return false;
		char c = knownCharValue.value();
		return (c >= 32 && c <= 126);
	}
};
class IRTokenPrintStr : public IRToken
{
	public:
	std::string str;
	IRTokenPrintStr(std::string& s)
	: str(s),
	IRToken()
	{
	}
	std::string generateCode() const override;
	
	std::string getName() const override
	{
		return "IRTokenPrintStr";
	}
};
class IRTokenMultiply : public IRToken
{
	public:
	int cellsAway;
	int factor;
	bool doClear;
	IRTokenMultiply(int _cellsAway, int _factor) 
	: cellsAway(_cellsAway), factor(_factor), IRToken(){}
	std::string getName() const override
	{
		return "IRTokenMultiply";
	}
	std::string generateCode() const override;
};

void convertTokensToIR(std::vector<Token*>& pTokensVec, std::vector<IRToken*>& pIRTokensVec);
void optimizeIRTokens(std::vector<IRToken*>& pIRTokensVec);
void printIRTokens( std::vector<IRToken*>& pIRTokensVec);

