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
		std::string name = "MultiAdd(";
		name+=std::to_string(intVal);
		name+=")";
		return name;
	}
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
		std::string name = "IRTokenMultiShift(";
		name+=std::to_string(numShifts);
		name+=")";
		return name;
	}
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
};
void convertTokensToIR(std::vector<Token*>& pTokensVec, std::vector<IRToken*>& pIRTokensVec);
void printIRTokens( std::vector<IRToken*>& pIRTokensVec);

