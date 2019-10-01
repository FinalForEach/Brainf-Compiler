#include "IR.hpp"
#include <set>
#include <vector>
#include <map>
#include <algorithm>
void optimizeIRTokensMultiplyPass(std::vector<IRToken*>& pIRTokensVec)
{
	for(unsigned int ti=0;ti<pIRTokensVec.size();ti++)
	{
		IRTokenLoopOpen *pirTokenLoopOpen = dynamic_cast<IRTokenLoopOpen*>(pIRTokensVec[ti]);
		if(pirTokenLoopOpen!=nullptr)
		{
			unsigned int w=ti+1;
			IRToken *pirToken = pIRTokensVec[w];
			bool isMultPattern=false;
			bool foundDests=false;
			int scope=0;
			IRTokenMultiAdd *decMadd;
			std::vector<IRToken*> irPatternTokens;
			while(w<pIRTokensVec.size())//Check if matches multiply pattern
			{
				pirToken = pIRTokensVec[w];
				
				if(dynamic_cast<IRTokenMultiShift*>(pirToken)!=nullptr
				||dynamic_cast<IRTokenMultiply*>(pirToken)!=nullptr)
				{
					isMultPattern=false;
					break;//Give up for now
				}
				if(dynamic_cast<IRTokenLoopClose*>(pirToken)!=nullptr )
				{
					if(foundDests&&decMadd!=nullptr)isMultPattern=true;
					break;
				}
				scope+=pirToken->getScope();
				if(scope!=0)
				{
					break;
				}
				irPatternTokens.push_back(pirToken);
				IRTokenMultiAdd *madd = dynamic_cast<IRTokenMultiAdd*>(pirToken);
				if(madd!=nullptr)
				{
					if(madd->cellsAway==pirTokenLoopOpen->cellsAway)
					{//If the condition decrements.
						if(madd->intVal==-1)
						{
							decMadd=madd;
							irPatternTokens.pop_back();
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
				pIRTokensVec[ti]=new IRTokenIfOpen(pirTokenLoopOpen->cellsAway);
				
				int noOpR;
				int lastMultR;
				unsigned int r =ti;
				//for(unsigned int r=ti+1;r<pIRTokensVec.size();r++)
				std::reverse(std::begin(irPatternTokens), std::end(irPatternTokens));
				while(!irPatternTokens.empty())
				{
					r++;
					IRToken *curToken = irPatternTokens.back();
					irPatternTokens.pop_back();
					IRTokenMultiAdd *madd = dynamic_cast<IRTokenMultiAdd*>(curToken);
					if(madd!=nullptr)
					{
						if(madd->cellsAway!=pirTokenLoopOpen->cellsAway)
						{
							auto *newMult=new IRTokenMultiply(madd->cellsAway,madd->intVal);
							newMult->factorACellsAway=pirTokenLoopOpen->cellsAway;
							pIRTokensVec[r]=newMult;
							lastMultR=r;
							
							std::cout<<"Replacing "<<madd->getName()<<"#"<<madd->getIRTokenID()<<" with mult#"<<pIRTokensVec[r]->getIRTokenID()<<"\n";
							continue;
						}
					}
					if(dynamic_cast<IRTokenLoopClose*>(curToken)!=nullptr
					||dynamic_cast<IRTokenLoopOpen*>(curToken)!=nullptr)
					{
						break;
					}
					pIRTokensVec[r]=curToken;//For the rest of the tokens.
				}
				
				
				pIRTokensVec[++r]=new IRTokenClear(0,decMadd->cellsAway);
				
				IRTokenIfClose *closingIfBracket =new IRTokenIfClose(false);
				closingIfBracket->offsetCells(pirTokenLoopOpen->cellsAway);
				pIRTokensVec[w]=closingIfBracket;
				
				
			}
		}
		
	}
}

void optimizeIRTokensComplexLoops(std::vector<IRToken*>& pIRTokensVec)
{
	//Can reduce loops to an if statement if:
	//Only adds, mults, and clears, conditional value is decremented,
	//and all multiply factors are cleared after
	for(unsigned int ti=0;ti<pIRTokensVec.size();ti++)
	{
		IRToken *irToken = pIRTokensVec[ti];
		IRTokenLoopOpen *irLoop = dynamic_cast<IRTokenLoopOpen*>(irToken);
		if(irLoop!=nullptr)
		{
			std::cout<<"Checking if irLoop#"<<irLoop->getIRTokenID()<<" isReduceable:\n";
			unsigned int s=ti+1;
			int scope=0;
			bool isReduceable=false;
			int condSetToZero=false;
			std::set<int> cellsToBeCleared;
			std::set<int> cellsNeedingClearing;
			std::vector<IRToken*> irs;
			std::vector<IRToken*> clears;//Condition must be set AFTER the rest.
			for(;s<pIRTokensVec.size();s++)
			{
				IRToken *irTokenS = pIRTokensVec[s];
				scope+=irTokenS->getScope();
				if(scope>0)
				{
					isReduceable=false;break;
				}
				if(scope<0)
				{
					isReduceable=cellsToBeCleared.empty();
					break;
				}
				auto *irTokenClear = dynamic_cast<IRTokenClear*>(irTokenS);
				auto *irTokenMult = dynamic_cast<IRTokenMultiply*>(irTokenS);
				auto *irTokenAdd = dynamic_cast<IRTokenMultiAdd*>(irTokenS);
				if(irTokenClear!=nullptr)
				{
					if(irTokenClear->setVal==0)
					{
						cellsToBeCleared.erase(irTokenClear->cellsAway);
						if(irTokenClear->cellsAway==irLoop->cellsAway)
						{
							condSetToZero=true;
						}
					}
				}
				if(irTokenMult!=nullptr)
				{
					cellsToBeCleared.insert(irTokenMult->factorACellsAway);
					cellsNeedingClearing.insert(irTokenMult->factorACellsAway);
					
					if(cellsNeedingClearing.find(irTokenMult->cellsAway)!=cellsNeedingClearing.end())
					{
						cellsToBeCleared.insert(irTokenMult->cellsAway);
					}
				}
				if(irTokenAdd!=nullptr)
				{
					if(irTokenAdd->cellsAway==irLoop->cellsAway){
						if(irTokenAdd->intVal!=-1){
							isReduceable=false;break;
						}else
						{
							clears.push_back(irTokenAdd);
						}
					}else
					{
						if(cellsNeedingClearing.find(irTokenAdd->cellsAway)!=cellsNeedingClearing.end())
						{
							cellsToBeCleared.insert(irTokenAdd->cellsAway);
						}
						irs.push_back(irTokenS);
					}
				}else
				{
					irs.push_back(irTokenS);
				}
				if(irTokenClear==nullptr && irTokenMult==nullptr && irTokenAdd ==nullptr
				&& dynamic_cast<IRTokenNoOp*>(irTokenS)==nullptr){
					isReduceable=false;break;
				}
			}
			
			if(isReduceable)
			{
				unsigned int x=ti+1;
				for(auto *ir : irs)
				{
					auto *irTokenAdd = dynamic_cast<IRTokenMultiAdd*>(ir);
					if(irTokenAdd!=nullptr)
					{
						if(condSetToZero)
						{
							pIRTokensVec[x++]=irTokenAdd;
						}else
						{
							auto redMult = new IRTokenMultiply(irTokenAdd->cellsAway,irTokenAdd->intVal);
							redMult->factorACellsAway=irLoop->cellsAway;
							pIRTokensVec[x++]=redMult;
						}
					}else
					{
						pIRTokensVec[x++]=ir;	
					}
				}
				for(auto *clr : clears)
				{
					auto *irTokenClear = dynamic_cast<IRTokenClear*>(clr);
					auto *irTokenAdd = dynamic_cast<IRTokenMultiAdd*>(clr);
					if(irTokenAdd!=nullptr)
					{
						pIRTokensVec[x++]=new IRTokenClear(0,irTokenAdd->cellsAway);	
						continue;
					}
					if(irTokenClear!=nullptr)
					{
						pIRTokensVec[x++]=irTokenClear;
					}
				}
				pIRTokensVec[ti]= new IRTokenIfOpen(irLoop->cellsAway);
				pIRTokensVec[s]= new IRTokenIfClose(false,irLoop->cellsAway);
			}
		}
	}
}
