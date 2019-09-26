#include "IR.hpp"
#include "Environment.hpp"
void optimizeIRTokensMultiplyPass(std::vector<IRToken*>& pIRTokensVec)
{
	for(unsigned int ti=0;ti<pIRTokensVec.size();ti++)
	{
		IRToken *pirToken = pIRTokensVec[ti];
		if(pirToken->getName()=="IRTokenLoopOpen")
		{
			unsigned int w=ti+1;
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
				pIRTokensVec[ti]=new IRTokenIfOpen();
				int noOpR;
				int lastMultR;
				bool hasLastMult=false;
				for(unsigned int r=ti+1;r<pIRTokensVec.size();r++)
				{
					IRTokenMultiAdd *madd = dynamic_cast<IRTokenMultiAdd*>(pIRTokensVec[r]);
					if(madd!=nullptr)
					{
						if(madd->cellsAway!=0)
						{
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
void optimizeIRTokensKnownVals(std::vector<IRToken*>& pIRTokensVec)
{
	Environment env;
	int curIndex=0;
	for(unsigned int ti=0;ti<pIRTokensVec.size();ti++)
	{
		IRToken *irToken = pIRTokensVec[ti];
		IRTokenMultiAdd *madd = dynamic_cast<IRTokenMultiAdd*>(irToken);
		if(madd!=nullptr)
		{
			try
			{
				//env.knownCells.at(madd->cellsAway)+=madd->intVal;
			}catch(std::out_of_range& oor){}
		}
		IRTokenClear *irClear = dynamic_cast<IRTokenClear*>(irToken);
		if(irClear!=nullptr)
		{
			//env.knownCells[irClear->cellsAway]=irClear->setVal;
		}
		IRTokenMultiShift *mshift = dynamic_cast<IRTokenMultiShift*>(irToken);
		if(mshift!=nullptr)
		{
			curIndex+=mshift->numShifts;
		}
		IRTokenLoopOpen *irLoopOpen = dynamic_cast<IRTokenLoopOpen*>(irToken);
		IRTokenLoopClose *irLoopClose = dynamic_cast<IRTokenLoopClose*>(irToken);
		if(irLoopOpen!=nullptr)//Check for loop condition for dead-code removal
		{
			/*try
			{
				std::cout<<"Encountered LoopOpen\n";
				if(ti+1<pIRTokensVec.size())std::cout<<"\tBefore "<<pIRTokensVec[ti+1]->getName()<<"\n";
				if(ti>0)std::cout<<"\tAfter "<<pIRTokensVec[ti-1]->getName()<<"\n";
				int knownCondition = env.knownCells.at(curIndex);
				std::cout<<"\t\tknownCondition="<<knownCondition<<"\n";
				if(knownCondition==0)
				{
					pIRTokensVec[ti] = new IRTokenNoOp(pIRTokensVec[ti]);
					int loopCount=1;
					for(int l=ti+1;l<pIRTokensVec.size();l++)
					{
						IRToken *irTokenInLoop = pIRTokensVec[l];
						pIRTokensVec[l] = new IRTokenNoOp(pIRTokensVec[l]);//Dead code.
						if(dynamic_cast<IRTokenLoopOpen*>(irTokenInLoop)!=nullptr)
						{
							loopCount++;
						}
						if(dynamic_cast<IRTokenLoopClose*>(irTokenInLoop)!=nullptr)
						{
							loopCount--;
							if(loopCount==0)//Found matching loop bracket
							{
								break;
							}
						}
					}
				}
			}catch(std::out_of_range& oor){}*/
			env.knownCells.clear();//Forget everything.
			curIndex=0;//Make known cells relative to current position.
			std::cout<<"Cleared knownCells.\n";
		}
		if(irLoopClose!=nullptr)//In or exiting a loop.
		{
			//env.knownCells.clear();//Forget everything.
		//	curIndex=0;//Make known cells relative to current position.
			
			//env.knownCells[curIndex]=0;//Current must be zero to exit loop
			//std::cout<<"Cleared knownCells.\n";
			//std::cout<<"\tNow know knownCells["<<curIndex<<"]="<<env.knownCells[curIndex]<<" \n";
		}
		IRTokenInput *irInput = dynamic_cast<IRTokenInput*>(irToken);
		if(irInput!=nullptr)//Cannot know user input.
		{
			env.knownCells.erase(curIndex);
			std::cout<<"Cleared knownCells.\n";
		}
		IRTokenMultiply *irMult = dynamic_cast<IRTokenMultiply*>(irToken);
		if(irMult!=nullptr)
		{
			std::cout<<"Found irMult\n";
			std::cout<<"Checking curIndex="<<curIndex<<"\n";
			try //If factors are known, reduce to an add.
			{
				/*
				std::cout<<"\tFactor="<<irMult->factor<<"\n";
				int knownFactor = env.knownCells.at(curIndex);
				std::cout<<"\tknownFactor="<<knownFactor<<"\n";
				int result = knownFactor * irMult->factor;
				
				pIRTokensVec[ti] = new IRTokenMultiAdd(result,irMult->cellsAway);
				
				ti--;continue;//Replaced token, so track it
				*/
			}catch(std::out_of_range& oor){}
		}
		IRTokenIfOpen *irIfOpen = dynamic_cast<IRTokenIfOpen*>(irToken);
		if(irIfOpen!=nullptr)
		{
			/*try
			{
				int knownCondition = env.knownCells.at(curIndex);
				if(knownCondition==0)
				{
					pIRTokensVec[ti] = new IRTokenNoOp(pIRTokensVec[ti]);
					int ifCount=1;
					for(int l=ti;l<pIRTokensVec.size();l++)
					{
						IRToken *irTokenInIf = pIRTokensVec[l];
						if(dynamic_cast<IRTokenIfOpen*>(irTokenInIf)!=nullptr)
						{
							ifCount++;
						}
						if(dynamic_cast<IRTokenIfClose*>(irTokenInIf)!=nullptr)
						{
							ifCount--;
							if(ifCount==0)//Found matching if bracket
							{
								pIRTokensVec[l] = new IRTokenNoOp(pIRTokensVec[l]);
								break;
							}
						}
					}
				}
			}catch(std::out_of_range& oor){}*/
		}
	}
}
void optimizeIRTokens(std::vector<IRToken*>& pIRTokensVec)
{
	optimizeIRTokensMultiplyPass(pIRTokensVec);
	
	
	optimizeIRTokensKnownVals(pIRTokensVec);
	//optimizeIRTokensKnownVals(pIRTokensVec);
}
