#include "IR.hpp"
#include <map>

void addSectionIRTokens(std::vector<IRToken*>& pIRTokensVec, 
	std::map<int,int>& cellAdds, std::map<int,bool>& cellClears)
{
	for (const auto &p : cellClears) {//Clears and sets
        int cell = p.first;
        bool doesClear = p.second;
        if(doesClear)
        {
			int val = 0;
			try
			{
				val=cellAdds.at(cell);//Sets cell to add value
				cellAdds.erase(cell); //Removes as not to add again.
			}catch(const std::out_of_range& oor){}
			pIRTokensVec.push_back(new IRTokenClear(val, cell));
		}
    }
    cellClears.clear();
    for (const auto &p : cellAdds) {
        int cell = p.first;
        int val = p.second;
        pIRTokensVec.push_back(new IRTokenMultiAdd(val, cell));
	}
	cellAdds.clear();
}
void convertTokensToIR(std::vector<Token*>& pTokensVec, std::vector<IRToken*>& pIRTokensVec)
{
	Environment env;
	std::map<int,int> cellAdds;
	std::map<int,bool> cellClears;
	int plusMinusCount = 0;
	int shiftCount = 0;
	for(int ti=0;ti<pTokensVec.size();ti++)
	{
		Token *pToken = pTokensVec[ti];
		
		//Clear
		if(pTokensVec.size()-ti>=3 && pToken->getName() == "OPEN_BRACKET"){
			if(pTokensVec[ti+1]->getName() == "PLUS" ||pTokensVec[ti+1]->getName() == "MINUS"){
				if(pTokensVec[ti+2]->getName() == "CLOSE_BRACKET"){
					//pIRTokensVec.push_back(new IRTokenClear());
					cellClears[shiftCount]=true;
					cellAdds[shiftCount]=0;//Ignore adds before clears.
					ti+=2;//Consume tokens
					continue;
				}
			}
		}
		if(pToken->getName() == "PLUS")
		{
			cellAdds.try_emplace(shiftCount,0);//Add mapping if not yet existing
			cellAdds[shiftCount]++;
		}
		if(pToken->getName() == "MINUS")
		{
			cellAdds.try_emplace(shiftCount,0);//Add mapping if not yet existing
			cellAdds[shiftCount]--;
		}
		if(pToken->getName() == "SHIFT_RIGHT")
		{
			shiftCount++;
		}
		if(pToken->getName() == "SHIFT_LEFT")
		{
			shiftCount--;
		}
		
		/*//MultiAdd
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
		}*/
		/*
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
		}*/
		if(pToken->getName() == "OPEN_BRACKET"
			|| pToken->getName() == "CLOSE_BRACKET"
			|| pToken->getName() == "COMMA"
			|| pToken->getName() == "PERIOD")
		{
			addSectionIRTokens(pIRTokensVec, cellAdds, cellClears);
			
			if(shiftCount!=0)
			{
				pIRTokensVec.push_back(new IRTokenMultiShift(shiftCount));
			}
			
			plusMinusCount = 0;
			shiftCount = 0;
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
	optimizeIRTokens(pIRTokensVec, env);
}
void optimizeIRTokens(std::vector<IRToken*>& pIRTokensVec, Environment& env)
{
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
