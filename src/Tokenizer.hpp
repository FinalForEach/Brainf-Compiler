#pragma once
#include <vector>
#include <iostream>
#include <string>

class Token
{
	public:
	char value;
	Token(char c)
	:value(c)
	{}
	~Token(){}
	virtual std::string getName() const
	{
		return "UNKNOWN_TOKEN";
	}
};

class TokenPlus : public Token
{
	public:
	TokenPlus(): Token('+'){}
	std::string getName() const override
	{
		return "PLUS";
	}
};
class TokenMinus : public Token
{
	public:
	TokenMinus(): Token('-'){}
	std::string getName() const override
	{
		return "MINUS";
	}
};

class TokenShiftRight : public Token
{
	public:
	TokenShiftRight(): Token('>'){}
	std::string getName() const override
	{
		return "SHIFT_RIGHT";
	}
};
class TokenShiftLeft : public Token
{
	public:
	TokenShiftLeft(): Token('<'){}
	std::string getName() const override
	{
		return "SHIFT_LEFT";
	}
};

class TokenComma : public Token
{
	public:
	TokenComma(): Token(','){}
	std::string getName() const override
	{
		return "COMMA";
	}
};
class TokenPeriod : public Token
{
	public:
	TokenPeriod(): Token('.'){}
	std::string getName() const override
	{
		return "PERIOD";
	}
};

class TokenOpenB : public Token
{
	public:
	TokenOpenB(): Token('['){}
	std::string getName() const override
	{
		return "OPEN_BRACKET";
	}
};
class TokenCloseB : public Token
{
	public:
	TokenCloseB(): Token(']'){}
	std::string getName() const override
	{
		return "CLOSE_BRACKET";
	}
};

void tokenizeString(std::string& str, std::vector<Token*>& pTokensVec);

void printTokens( std::vector<Token*>& pTokensVec);
