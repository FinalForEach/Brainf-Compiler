#pragma once
#include <vector>
#include <iostream>
#include <string>
#include <optional>
#include "Tokenizer.hpp"



extern int numIrTokensInitialized;

class IRToken
{
	protected:
	int irTokenID;
	void setIRTokenID(int i)
	{
		irTokenID=i;
	}
	public:
	IRToken()
	{
		irTokenID=numIrTokensInitialized++;
	}
	~IRToken(){}
	virtual std::string getName() const
	{
		return "UNKNOWN_IRTOKEN";
	}
	virtual std::string generateCode() const = 0;
	virtual int getPreIndentModifier() const
	{
		return 0;
	}
	virtual int getPostIndentModifier() const
	{
		return 0;
	}
	virtual void offsetCells(int _cellsAway){}
	int getIRTokenID()
	{
		return irTokenID;
	}
	virtual int getScope() const
	{
		return 0;
	}
};
class IRTokenComment : public IRToken
{
	std::string commentStr;
	public:
	IRTokenComment(std::string str) : IRToken(), commentStr(str)
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
	IRTokenNoOp() : IRToken(), overridenIRToken(nullptr)
	{
	}
	IRTokenNoOp(IRToken *deletedIr) : IRToken(), overridenIRToken(deletedIr)
	{
		//Keep the id the same as the overriden one, for difference checking.
		setIRTokenID(overridenIRToken->getIRTokenID());
		if(dynamic_cast<IRTokenNoOp*>(deletedIr)==nullptr)
		{
			numIrTokensInitialized--;	
		}
	}
	std::string getName() const override
	{
		return "IRTokenNoOp";
	}
	std::string generateCode() const override
	{
		if(overridenIRToken!=nullptr)
		{
			std::string code = "//Overwritten ";
			code+=overridenIRToken->generateCode();
			code+=" //";
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
	IRTokenMultiAdd(int i): IRToken(), intVal(i), cellsAway(0)
	{
	}
	IRTokenMultiAdd(int i, int c): IRToken(), intVal(i), cellsAway(c)
	{
	}
	std::string getName() const override
	{
		return "IRTokenMultiAdd";
	}
	std::string generateCode() const override;
	void offsetCells(int _cellsAway) override
	{
		cellsAway+=_cellsAway;
	}
};
class IRTokenMultiShift : public IRToken
{
	public:
	int numShifts;
	IRTokenMultiShift(int i): IRToken(), numShifts(i)
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
	IRTokenClear(): IRToken(), setVal(0), cellsAway(0)
	{
	}
	IRTokenClear(int v, int c): IRToken(), setVal(v), cellsAway(c)
	{
	}
	std::string getName() const override
	{
		return "IRTokenClear";
	}
	std::string generateCode() const override;
	void offsetCells(int _cellsAway) override
	{
		cellsAway+=_cellsAway;
	}
};
class IRTokenLoopOpen : public IRToken
{
	public:
	int cellsAway;
	IRTokenLoopOpen() : IRToken(), cellsAway(0)
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
	void offsetCells(int _cellsAway) override
	{
		cellsAway+=_cellsAway;
	}
	int getScope() const override
	{
		return 1;
	}
};
class IRTokenLoopClose : public IRToken
{
	public:
	IRTokenLoopClose() : IRToken()
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
	int getScope() const override
	{
		return -1;
	}
};

class IRTokenIfOpen : public IRToken
{
	public:
	int cellsAway;
	IRTokenIfOpen() : IRToken(), cellsAway(0)
	{
	}
	IRTokenIfOpen(int c) : IRToken(), cellsAway(c)
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
	void offsetCells(int _cellsAway) override
	{
		cellsAway+=_cellsAway;
	}
	int getScope() const override
	{
		return 1;
	}
};
class IRTokenIfClose : public IRToken
{
	public:
	bool doesClear;
	int cellsAway;
	IRTokenIfClose(bool _doesClear) : IRToken(), doesClear(_doesClear), cellsAway(0)
	{
	}
	IRTokenIfClose(bool _doesClear, int c) : IRToken(), doesClear(_doesClear), cellsAway(c)
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
	void offsetCells(int _cellsAway) override
	{
		cellsAway+=_cellsAway;
	}
	int getScope() const override
	{
		return -1;
	}
};

class IRTokenInput : public IRToken
{
	public:
	int cellsAway;
	IRTokenInput() : IRToken()
	{
	}
	std::string getName() const override
	{
		return "IRTokenInput";
	}
	std::string generateCode() const override;
	void offsetCells(int _cellsAway) override
	{
		cellsAway+=_cellsAway;
	}
};
class IRTokenPrintChar : public IRToken
{
	public:
	std::optional<int> knownCharValue;
	int cellsAway;//Not required if char known
	IRTokenPrintChar(): IRToken(), cellsAway(0)
	{
	}
	IRTokenPrintChar(int c): IRToken(), knownCharValue(c), cellsAway(0)
	{
	}
	std::string getName() const override
	{
		return "IRTokenPrintChar";
	}
	std::string generateCode() const override;
	
	void offsetCells(int _cellsAway) override
	{
		cellsAway+=_cellsAway;
	}
	
	
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
	: IRToken(), str(s)
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
	int factorACellsAway;
	int factor;
	IRTokenMultiply(int _cellsAway, int _factor) 
	: IRToken(), cellsAway(_cellsAway),factorACellsAway(0), factor(_factor){}
	std::string getName() const override
	{
		return "IRTokenMultiply";
	}
	std::string generateCode() const override;
	void offsetCells(int _cellsAway) override
	{
		cellsAway+=_cellsAway;
		factorACellsAway+=_cellsAway;
	}
};

void convertTokensToIR(std::vector<Token*>& pTokensVec, std::vector<IRToken*>& pIRTokensVec);
void optimizeIRTokens(std::vector<IRToken*>& pIRTokensVec);
void optimizeIRTokensMultiplyPass(std::vector<IRToken*>& pIRTokensVec);
void optimizeIRTokensCancelShifts(std::vector<IRToken*>& pIRTokensVec);
void optimizeIRTokensDiffuseShifts(std::vector<IRToken*>& pIRTokensVec);
void optimizeIRTokensReduceMultiplyIfs(std::vector<IRToken*>& pIRTokensVec);
void optimizeIRTokensComplexLoops(std::vector<IRToken*>& pIRTokensVec);
void optimizeIRTokensKnownVals(std::vector<IRToken*>& pIRTokensVec);
void optimizeIRTokensCondenseSets(std::vector<IRToken*>& pIRTokensVec);
void ridOfNoOps(std::vector<IRToken*>& pIRTokensVec);
void printIRTokens( std::vector<IRToken*>& pIRTokensVec);

