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
	int curRelIndex=0;
	for(unsigned int ti=0;ti<pIRTokensVec.size();ti++)
	{
		IRToken *irToken = pIRTokensVec[ti];
		IRTokenMultiAdd *madd = dynamic_cast<IRTokenMultiAdd*>(irToken);
		if(madd!=nullptr)
		{
			try
			{
				int& cell = env.knownCells.at(madd->cellsAway + curRelIndex);
				cell+=madd->intVal;
			}catch(std::out_of_range& oor){}
		}
		IRTokenClear *irClear = dynamic_cast<IRTokenClear*>(irToken);
		if(irClear!=nullptr)
		{
			env.knownCells[irClear->cellsAway + curRelIndex]=irClear->setVal;
		}
		IRTokenMultiShift *mshift = dynamic_cast<IRTokenMultiShift*>(irToken);
		if(mshift!=nullptr)
		{
			curRelIndex+=mshift->numShifts;
		}
		IRTokenLoopOpen *irLoopOpen = dynamic_cast<IRTokenLoopOpen*>(irToken);
		IRTokenLoopClose *irLoopClose = dynamic_cast<IRTokenLoopClose*>(irToken);
		if(irLoopOpen!=nullptr)//Check for loop condition for dead-code removal
		{
			try
			{/*
				int knownCondition = env.knownCells.at(curRelIndex+irLoopOpen->cellsAway);
				std::cout<<"curRelIndex="<<curRelIndex<<"\n";
				std::cout<<"knownCells.at(curRelIndex)="<<knownCondition<<"\n";
				if(knownCondition==0)
				{
					pIRTokensVec[ti] = new IRTokenNoOp(pIRTokensVec[ti]);
					int loopCount=1;
					for(unsigned int l=ti+1;l<pIRTokensVec.size();l++)
					{
						IRToken *irTokenInLoop = pIRTokensVec[l];
						if(dynamic_cast<IRTokenComment*>(irTokenInLoop)==nullptr)
						{
							pIRTokensVec[l] = new IRTokenNoOp(pIRTokensVec[l]);//Dead code.
						}
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
				}*/
			}catch(std::out_of_range& oor){}
			env.knownCells.clear();//Forget everything.
			curRelIndex=0;//Make known cells relative to current position.
		}
		if(irLoopClose!=nullptr)//In or exiting a loop.
		{
			env.knownCells.clear();//Forget everything.
			curRelIndex=0;//Make known cells relative to current position.
			
			env.knownCells[curRelIndex]=0;//Current must be zero to exit loop
		}
		IRTokenInput *irInput = dynamic_cast<IRTokenInput*>(irToken);
		if(irInput!=nullptr)//Cannot know user input.
		{
			env.knownCells.erase(curRelIndex);
			//std::cout<<"Cleared knownCells.\n";
		}
		IRTokenMultiply *irMult = dynamic_cast<IRTokenMultiply*>(irToken);
		if(irMult!=nullptr)
		{
			/*try //If factors are known, reduce to an add.
			{
				
				const int knownFactor = env.knownCells.at(curRelIndex+irMult->factorACellsAway);
				std::cout<<"Found irMult#"<<irMult->getIRTokenID()<<"\n";
				std::cout<<"Checking curRelIndex="<<curRelIndex+irMult->factorACellsAway<<"\n";
				std::cout<<"\t"<<env.knownCellsToString()<<"\n";
				std::cout<<"\tFactor="<<irMult->factor<<"\n";
				std::cout<<"\tknownFactor="<<knownFactor<<"\n";
				const int result = knownFactor * irMult->factor;
				std::cout<<"\tresult="<<result<<"\n";
				if(result==0)
				{
					std::cout<<"\tWarning, result is 0\n";
				}
				pIRTokensVec[ti] = new IRTokenMultiAdd(result,irMult->cellsAway);
				std::cout<<"\tReplaced with IRTokenMultiAdd#"<<pIRTokensVec[ti]->getIRTokenID()<<"\n";
				ti--;continue;//Replaced token, so track it
				
				
			}catch(std::out_of_range& oor)
			{
				//If unknown factors, mark current cell as unknown.
				env.knownCells.erase(curRelIndex+irMult->cellsAway);
			}*/
		}
		IRTokenIfOpen *irIfOpen = dynamic_cast<IRTokenIfOpen*>(irToken);
		if(irIfOpen!=nullptr)
		{
			try
			{
				/*
				std::cout<<"irIfOpen->cellsAway="<<irIfOpen->cellsAway<<"\n";
				int knownCondition = env.knownCells.at(curIndex+irIfOpen->cellsAway);
				if(knownCondition==0)//Known false, so remove dead code.
				{
					pIRTokensVec[ti] = new IRTokenNoOp(pIRTokensVec[ti]);
					int ifCount=1;
					for(unsigned int l=ti+1;l<pIRTokensVec.size();l++)
					{
						IRToken *irTokenInIf = pIRTokensVec[l];
						if(dynamic_cast<IRTokenComment*>(irTokenInIf)==nullptr)
						{
							pIRTokensVec[l] = new IRTokenNoOp(pIRTokensVec[l]);//Dead code.
						}
						if(dynamic_cast<IRTokenIfOpen*>(irTokenInIf)!=nullptr)
						{
							ifCount++;
						}
						if(dynamic_cast<IRTokenIfClose*>(irTokenInIf)!=nullptr)
						{
							ifCount--;
							if(ifCount==0)//Found matching if bracket
							{
								break;
							}
						}
					}
				}else //Known true, so rid of brackets
				{
					pIRTokensVec[ti] = new IRTokenNoOp(pIRTokensVec[ti]);//Redundant code
					int ifCount=1;
					for(unsigned int l=ti+1;l<pIRTokensVec.size();l++)
					{
						IRToken *irTokenInLoop = pIRTokensVec[l];
						if(dynamic_cast<IRTokenIfOpen*>(irTokenInLoop)!=nullptr)
						{
							ifCount++;
						}
						if(dynamic_cast<IRTokenIfClose*>(irTokenInLoop)!=nullptr)
						{
							ifCount--;
							if(ifCount==0)//Found matching if bracket
							{
								pIRTokensVec[l] = new IRTokenNoOp(pIRTokensVec[l]);//Redundant code
								break;
							}
						}
					}
				}*/
			}catch(std::out_of_range& oor){}
		}
	}
}
void optimizeIRTokensOffsetShifts(std::vector<IRToken*>& pIRTokensVec)
{
	for(unsigned int ti=0;ti<pIRTokensVec.size();ti++)
	{
		IRToken *irToken = pIRTokensVec[ti];
		IRTokenMultiShift *irShift = dynamic_cast<IRTokenMultiShift*>(irToken);
		if(irShift!=nullptr)
		{
			int loopCount=0;
			int ifCount=0;
			bool isBalanced=false;
			unsigned int s=ti+1;
			for(;s<pIRTokensVec.size();s++)
			{
				IRToken *irTokenS = pIRTokensVec[s];
				if(dynamic_cast<IRTokenLoopOpen*>(irTokenS))
					loopCount++;
				if(dynamic_cast<IRTokenLoopClose*>(irTokenS))
					loopCount--;
				if(dynamic_cast<IRTokenIfOpen*>(irTokenS))
					ifCount++;
				if(dynamic_cast<IRTokenIfClose*>(irTokenS))
					ifCount--;
				if(ifCount<0 || loopCount<0)
				{
					break;//Out of scope
				}
				IRTokenMultiShift *irShiftS = dynamic_cast<IRTokenMultiShift*>(irTokenS);
				if(irShiftS!=nullptr && loopCount==0 && ifCount==0)
				{
					isBalanced = irShift->numShifts == -irShiftS->numShifts;
					break;
				}
			}
			if(isBalanced)
			{
				for(unsigned int o=ti+1;o<s;o++)
				{
					pIRTokensVec[o]->offsetCells(irShift->numShifts);
				}
				pIRTokensVec[ti] = new IRTokenNoOp(pIRTokensVec[ti]);
				pIRTokensVec[s] = new IRTokenNoOp(pIRTokensVec[s]);
			}
		}
	}
}
void ridOfNoOps(std::vector<IRToken*>& pIRTokensVec)
{
	//Traverse backwards, to maintain iteration
	for(int ti=pIRTokensVec.size()-1;ti>=0;ti--)
	{
		IRToken *irToken = pIRTokensVec[ti];
		if(dynamic_cast<IRTokenNoOp*>(irToken)!=nullptr)
		{
			pIRTokensVec.erase(pIRTokensVec.begin()+ti);
		}
	}
}
void optimizeIRTokens(std::vector<IRToken*>& pIRTokensVec)
{
	optimizeIRTokensMultiplyPass(pIRTokensVec);
	optimizeIRTokensOffsetShifts(pIRTokensVec);
	
	optimizeIRTokensKnownVals(pIRTokensVec);
	
	ridOfNoOps(pIRTokensVec);
	
}
