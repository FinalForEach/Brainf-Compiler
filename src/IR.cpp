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
	
	int knownValue=0;
	bool doesKnowValue=true;
	std::string printStr = "";
	
	for(int ti=0;ti<pTokensVec.size();ti++)
	{
		Token *pToken = pTokensVec[ti];
		
		//Clear
		if(pTokensVec.size()-ti>=3 && pToken->getName() == "OPEN_BRACKET"){
			if(pTokensVec[ti+1]->getName() == "PLUS" ||pTokensVec[ti+1]->getName() == "MINUS"){
				if(pTokensVec[ti+2]->getName() == "CLOSE_BRACKET"){
					cellClears[shiftCount]=true;
					cellAdds[shiftCount]=0;//Ignore adds before clears.
					knownValue=0;//Now know value at current cell to be zero
					ti+=2;//Consume tokens
					continue;
				}
			}
		}

		
		if(pToken->getName() == "PLUS")
		{
			cellAdds.try_emplace(shiftCount,0);//Add mapping if not yet existing
			cellAdds[shiftCount]++;
			if(shiftCount==0)knownValue++;
		}
		if(pToken->getName() == "MINUS")
		{
			cellAdds.try_emplace(shiftCount,0);//Add mapping if not yet existing
			cellAdds[shiftCount]--;
			if(shiftCount==0)knownValue--;
		}
		if(pToken->getName() == "SHIFT_RIGHT")shiftCount++;
		if(pToken->getName() == "SHIFT_LEFT")shiftCount--;

		
		if(doesKnowValue && pToken->getName() == "PERIOD")
		{
			if(knownValue >= 32 && knownValue <= 126)
			{
				printStr+=(char)knownValue;
			}else
			{
				if(printStr!="")
				{
					IRTokenPrintStr *irPrintStr = new IRTokenPrintStr(printStr);
					pIRTokensVec.push_back(irPrintStr);
					printStr="";
				}
				IRTokenPrintChar *irPrintChar = new IRTokenPrintChar(knownValue);
				pIRTokensVec.push_back(irPrintChar);
			}
			
		}
		if(pToken->getName() == "OPEN_BRACKET"
			|| pToken->getName() == "CLOSE_BRACKET"
			|| pToken->getName() == "COMMA"
			|| (pToken->getName() == "PERIOD" && !doesKnowValue))
		{
			addSectionIRTokens(pIRTokensVec, cellAdds, cellClears);
			
			if(shiftCount!=0)
			{
				pIRTokensVec.push_back(new IRTokenMultiShift(shiftCount));
			}
			
			plusMinusCount = 0;
			shiftCount = 0;
			doesKnowValue=false;
			
			if(printStr!="")
			{
				IRTokenPrintStr *irPrintChar = new IRTokenPrintStr(printStr);
				pIRTokensVec.push_back(irPrintChar);
				printStr="";
			}
			if((pToken->getName() == "PERIOD" && !doesKnowValue))
			{
				IRTokenPrintChar *irPrintChar = new IRTokenPrintChar();
				pIRTokensVec.push_back(irPrintChar);
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
		
		if(pToken->getName() == "COMMA")//Input
		{
			pIRTokensVec.push_back(new IRTokenInput());
		}
	}
	if(printStr!="")//Flush remaining prints.
	{
		IRTokenPrintStr *irPrintChar = new IRTokenPrintStr(printStr);
		pIRTokensVec.push_back(irPrintChar);
		printStr="";
	}
	printIRTokens(pIRTokensVec);
	optimizeIRTokens(pIRTokensVec, env);
}
void optimizeIRTokens(std::vector<IRToken*>& pIRTokensVec, Environment& env)
{
	return;
	for(int ti=0;ti<pIRTokensVec.size();ti++)
	{
		IRToken *pirToken = pIRTokensVec[ti];
		if(pirToken->getName()=="IRTokenLoopOpen")
		{
			int w=ti+1;
			std::map<int,int> madds;
			pirToken = pIRTokensVec[w];
			bool isMultPattern=false;
			bool foundDec=false;
			bool foundDests=false;
			while(w<pIRTokensVec.size())//Check if matches multiply pattern
			{
				pirToken = pIRTokensVec[w];
				if(pirToken->getName()=="IRTokenLoopOpen" || pirToken->getName()=="IRTokenMultiShift")
				{
					isMultPattern=false;
					break;//Give up for now
				}
				if(pirToken->getName()=="IRTokenLoopClose")
				{
					if(foundDests&&foundDec)isMultPattern=true;
					break;
				}
				IRTokenMultiAdd *madd = dynamic_cast<IRTokenMultiAdd*>(pirToken);
				if(madd!=nullptr)
				{
					if(madd->cellsAway==0)
					{
						if(madd->intVal==-1)
						{
							foundDec=true;
						}else
						{							
							isMultPattern=false;
							break;
						}
					}else
					{
						foundDests=true;
					}
				}
				w++;
			}
			if(isMultPattern)//Multiplication detected.
			{
				std::cout<<"(First)Replacing ["<<ti<<"]"<<pIRTokensVec[ti]->getName()<<" with ifOpen\n";
				pIRTokensVec[ti]=new IRTokenIfOpen();
				int noOpR;
				int lastMultR;
				bool hasLastMult=false;
				for(int r=ti+1;r<pIRTokensVec.size();r++)
				{
					IRTokenMultiAdd *madd = dynamic_cast<IRTokenMultiAdd*>(pIRTokensVec[r]);
					if(madd!=nullptr)
					{
						if(madd->cellsAway!=0)
						{
							std::cout<<"(For)Replacing ["<<r<<"]"<<pIRTokensVec[r]->getName()<<" with mult\n";
							pIRTokensVec[r]=new IRTokenMultiply(madd->cellsAway,madd->intVal);
							lastMultR=r;
							hasLastMult=true;
						}else
						{
							noOpR=r;
						}
					}
					if(dynamic_cast<IRTokenLoopClose*>(pIRTokensVec[r])!=nullptr
					||dynamic_cast<IRTokenLoopOpen*>(pIRTokensVec[r])!=nullptr)
					{
						break;
					}
				}
				if(hasLastMult)
				{
					if(noOpR<lastMultR)
					{
						//Swap last multiply and clearing decrement
						pIRTokensVec[noOpR]=pIRTokensVec[lastMultR];
						pIRTokensVec[lastMultR]= new IRTokenClear();	
					}else
					{
						pIRTokensVec[noOpR]= new IRTokenClear();
					}
					pIRTokensVec[w]=new IRTokenIfClose(false);
				}else
				{
					pIRTokensVec[noOpR]=new IRTokenNoOp(pIRTokensVec[noOpR]);
					pIRTokensVec[w]=new IRTokenIfClose(true);
				}
				
			}
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
