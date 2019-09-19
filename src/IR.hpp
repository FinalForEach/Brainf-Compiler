#pragma once
#include <vector>
#include <iostream>
#include <string>
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
class IRTokenMultiAdd : public IRToken
{
	public:
	int intVal;
	IRTokenMultiAdd(int i)
	: intVal(i),
	IRToken()
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
	IRTokenClear()
	: IRToken()
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
	IRTokenPrintChar()
	: IRToken()
	{
	}
	std::string getName() const override
	{
		return "IRTokenPrintChar";
	}
	std::string generateCode() const override;
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
void optimizeIRTokens(std::vector<IRToken*>& pIRTokensVec);
void printIRTokens( std::vector<IRToken*>& pIRTokensVec);

