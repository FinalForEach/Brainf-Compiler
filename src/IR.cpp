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
	optimizeIRTokens(pIRTokensVec);
}

void optimizeIRTokens(std::vector<IRToken*>& pIRTokensVec)
{
	std::vector<IRToken*> pIRTokensVecTmp;
	for(int ti=0;ti<pIRTokensVec.size();ti++)
	{
		IRToken *pIRToken = pIRTokensVec[ti];
		
		if(pIRTokensVec.size()-ti>=6 && pIRToken->getName() == "IRTokenLoopOpen")
		{
			if(pIRTokensVec[ti+1]->getName() == "IRTokenMultiShift")
			{
				if(pIRTokensVec[ti+2]->getName() == "IRTokenMultiAdd")
				{
					if(pIRTokensVec[ti+3]->getName() == "IRTokenMultiShift")
					{
						IRTokenMultiShift *shiftAway= dynamic_cast<IRTokenMultiShift*>(pIRTokensVec[ti+1]);
						IRTokenMultiShift *shiftBack= dynamic_cast<IRTokenMultiShift*>(pIRTokensVec[ti+3]);
						
						if(shiftAway->numShifts == -(shiftBack->numShifts)
						&& pIRTokensVec[ti+4]->getName() == "IRTokenMultiAdd")
						{
							IRTokenMultiAdd *decToken = dynamic_cast<IRTokenMultiAdd*>(pIRTokensVec[ti+4]);
							if(decToken->intVal==-1
							&& pIRTokensVec[ti+5]->getName() == "IRTokenLoopClose")
							{
								IRTokenMultiAdd *factorToken = dynamic_cast<IRTokenMultiAdd*>(pIRTokensVec[ti+2]);
								pIRTokensVecTmp.push_back(new IRTokenMultiply(shiftAway->numShifts,factorToken->intVal));
								ti+=5;//Consume ir tokens
								continue;
							}
						}
					}
				}
			}
		}
		pIRTokensVecTmp.push_back(pIRToken);
		
	}
	pIRTokensVec.clear();
	for(int i=0; i<pIRTokensVecTmp.size();i++)
	{
		pIRTokensVec.push_back(pIRTokensVecTmp[i]);
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
