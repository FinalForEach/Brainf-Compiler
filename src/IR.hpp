#pragma once
#include <vector>
#include <iostream>
#include <string>
#include <optional>
#include "Tokenizer.hpp"


class Environment 
{
	std::optional<int> knownCellValue;
	public:
	Environment()
	:knownCellValue(0)
	{
	}
	bool hasKnownCellValue() const
	{
		return knownCellValue.has_value();
	}
	int getKnownCellValue() const
	{
		return knownCellValue.value();
	}
	void addToKnownCellValue(int a)
	{
		knownCellValue.value()+=a;
	}
	void forgetCellValue()
	{
		knownCellValue=std::nullopt;
	}
	void rememberCellValue(int v)
	{
		knownCellValue=v;
	}
};

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
		std::string name ="IRTokenMultiAdd";
		name+="[";
		name+=std::to_string(intVal);
		name+="@";
		name+=std::to_string(cellsAway);
		name+="]";
		return name;
		//return "IRTokenMultiAdd";
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
	int setVal;
	int cellsAway;
	public:
	IRTokenClear(): cellsAway(0), setVal(0),IRToken()
	{
	}
	IRTokenClear(int v, int c): cellsAway(c), setVal(v),IRToken()
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
	IRTokenMultiply(int _cellsAway,int _factor)
	: 
	cellsAway(_cellsAway),
	factor(_factor),
	IRToken()
	{
	}
	std::string getName() const override
	{
		return "IRTokenMultiply";
	}
	std::string generateCode() const override;
};

void convertTokensToIR(std::vector<Token*>& pTokensVec, std::vector<IRToken*>& pIRTokensVec);
void optimizeIRTokens(std::vector<IRToken*>& pIRTokensVec, Environment& env);
void printIRTokens( std::vector<IRToken*>& pIRTokensVec);

