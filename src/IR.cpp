#include "IR.hpp"
#include <map>
#include "Environment.hpp"
int numIrTokensInitialized=0;
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
	std::map<int,int> cellAdds;
	std::map<int,bool> cellClears;
	int shiftCount = 0;
	
	std::string printStr = "";
	
	std::string commentStr = "";
	std::string whitespaceCommentStr = "";
	std::vector<IRTokenLoopOpen*> opens;
	for(unsigned int ti=0;ti<pTokensVec.size();ti++)
	{
		Token *pToken = pTokensVec[ti];
		if(pToken->getName()=="UNKNOWN_TOKEN")
		{
			if(!isspace(pToken->value))
			{
				if(whitespaceCommentStr!="")//Don't include whitespace unless required
				{
					if(commentStr!="")//Ignore whitespace before comment.
					{
						commentStr+=whitespaceCommentStr;
					}
					whitespaceCommentStr="";
				}
				commentStr+=pToken->value;
			}else
			{
				whitespaceCommentStr+=pToken->value;
			}
		}else
		{
			if(commentStr!="")
			{
				pIRTokensVec.push_back(new IRTokenComment(commentStr));
				commentStr="";
				whitespaceCommentStr="";
			}
		}
		
		//Clear
		if(pTokensVec.size()-ti>=3 && pToken->getName() == "OPEN_BRACKET"){
			if(pTokensVec[ti+1]->getName() == "PLUS" ||pTokensVec[ti+1]->getName() == "MINUS"){
				if(pTokensVec[ti+2]->getName() == "CLOSE_BRACKET"){
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
		if(pToken->getName() == "SHIFT_RIGHT")shiftCount++;
		if(pToken->getName() == "SHIFT_LEFT")shiftCount--;

		
		if(pToken->getName() == "OPEN_BRACKET"
			|| pToken->getName() == "CLOSE_BRACKET"
			|| pToken->getName() == "COMMA"
			|| (pToken->getName() == "PERIOD")
			|| pToken->getName() == "END_TOKEN")
		{
			addSectionIRTokens(pIRTokensVec, cellAdds, cellClears);
			
			if(shiftCount!=0)
			{
				pIRTokensVec.push_back(new IRTokenMultiShift(shiftCount));
			}
			
			shiftCount = 0;
			
			if(printStr!="")
			{
				IRTokenPrintStr *irPrintChar = new IRTokenPrintStr(printStr);
				pIRTokensVec.push_back(irPrintChar);
				printStr="";
			}
			if(pToken->getName() == "PERIOD")
			{
				IRTokenPrintChar *irPrintChar = new IRTokenPrintChar();
				pIRTokensVec.push_back(irPrintChar);
			}
		}

		
		//Loops
		if(pToken->getName() == "OPEN_BRACKET")
		{
			auto *lo = new IRTokenLoopOpen();
			pIRTokensVec.push_back(lo);
			opens.push_back(lo);
		}
		if(pToken->getName() == "CLOSE_BRACKET")
		{
			pIRTokensVec.push_back(new IRTokenLoopClose(opens.back()));
			opens.pop_back();
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
	optimizeIRTokens(pIRTokensVec);
}

void printIRTokens(std::vector<IRToken*>& pIRTokensVec)
{
	std::cout<<"{ ";
	for(unsigned int i=0; i<pIRTokensVec.size();i++)
	{
		std::cout<<pIRTokensVec[i]->getName()<<", ";
	}
	std::cout<<"}\n";
}
