#include "Tokenizer.hpp"



void tokenizeString(std::string& str, std::vector<Token*>& pTokensVec)
{
	for(int i=0;i<str.length();i++)
	{
		char c = str[i];
		Token *pToken;
		switch(c)
		{
			case '+':
			pToken = new TokenPlus();
			break;
			case '-':
			pToken = new TokenMinus();
			break;
			
			case '>':
			pToken = new TokenShiftRight();
			break;
			case '<':
			pToken = new TokenShiftLeft();
			break;
			
			case '[':
			pToken = new TokenOpenB();
			break;
			case ']':
			pToken = new TokenCloseB();
			break;
			
			case ',':
			pToken = new TokenComma();
			break;
			case '.':
			pToken = new TokenPeriod();
			break;
			
			default:
			pToken = new Token(c);
		}
		
		pTokensVec.push_back(pToken);
	}
}
void printTokens( std::vector<Token*>& pTokensVec)
{
	std::cout<<"{ ";
	for(int i=0; i<pTokensVec.size();i++)
	{
		std::cout<<pTokensVec[i]->getName()<<", ";
	}
	std::cout<<"}\n";
}