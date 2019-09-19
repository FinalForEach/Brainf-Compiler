#include "IR.hpp"

void convertTokensToIR(std::vector<Token*>& pTokensVec, std::vector<IRToken*>& pIRTokensVec)
{
	for(int ti=0;ti<pTokensVec.size();ti++)
	{
		Token *pToken = pTokensVec[ti];
		int plusMinusCount = 0;
		int shiftCount = 0;
		
		//MultiAdd
		while(pToken->getName() == "PLUS" || pToken->getName() == "MINUS")
		{
			if(pToken->getName() == "PLUS"){
				plusMinusCount++;
			}else{
				plusMinusCount--;
			}
			ti++;if(ti>=pTokensVec.size())break;
			
			pToken = pTokensVec[ti];
		}
		if(plusMinusCount!=0)
		{
			pIRTokensVec.push_back(new IRTokenMultiAdd(plusMinusCount));
			ti--;//So that a token is not skipped.
			continue;
		}
		
		//MultiShift
		while(pToken->getName() == "SHIFT_RIGHT" || pToken->getName() == "SHIFT_LEFT")
		{
			if(pToken->getName() == "SHIFT_RIGHT"){
				shiftCount++;
			}else{
				shiftCount--;
			}
			ti++;if(ti>=pTokensVec.size())break;
			
			pToken = pTokensVec[ti];
		}
		if(shiftCount!=0)
		{
			pIRTokensVec.push_back(new IRTokenMultiShift(shiftCount));
			ti--;//So that a token is not skipped.
			continue;
		}
		
		//Clear
		if(pTokensVec.size()-ti>=3 && pToken->getName() == "OPEN_BRACKET"){
			if(pTokensVec[ti+1]->getName() == "PLUS" ||pTokensVec[ti+1]->getName() == "MINUS"){
				if(pTokensVec[ti+2]->getName() == "CLOSE_BRACKET"){
					pIRTokensVec.push_back(new IRTokenClear());
					ti+=2;//Consume tokens
					continue;
				}
			}
		}
		
		//Loops
		if(pToken->getName() == "OPEN_BRACKET")
		{
			pIRTokensVec.push_back(new IRTokenLoopOpen());
		}
		if(pToken->getName() == "CLOSE_BRACKET")
		{
			pIRTokensVec.push_back(new IRTokenLoopClose());
		}
		
		//Input + Output
		if(pToken->getName() == "COMMA")
		{
			pIRTokensVec.push_back(new IRTokenInput());
		}
		if(pToken->getName() == "PERIOD")
		{
			pIRTokensVec.push_back(new IRTokenPrintChar());
		}
	}
}

void printIRTokens(std::vector<IRToken*>& pIRTokensVec)
{
	std::cout<<"{ ";
	for(int i=0; i<pIRTokensVec.size();i++)
	{
		std::cout<<pIRTokensVec[i]->getName()<<", ";
	}
	std::cout<<"}\n";
}
