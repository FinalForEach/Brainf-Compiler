#include "IR.hpp"
#include "Environment.hpp"
void optimizeIRTokensMultiplyPass(std::vector<IRToken*>& pIRTokensVec)
{
	for(unsigned int ti=0;ti<pIRTokensVec.size();ti++)
	{
		IRTokenLoopOpen *pirTokenLoopOpen = dynamic_cast<IRTokenLoopOpen*>(pIRTokensVec[ti]);
		if(pirTokenLoopOpen!=nullptr)
		{
			unsigned int w=ti+1;
			std::map<int,int> madds;
			IRToken *pirToken = pIRTokensVec[w];
			bool isMultPattern=false;
			bool foundDec=false;
			bool foundDests=false;
			bool ifsRequired=true;
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
					if(madd->cellsAway==pirTokenLoopOpen->cellsAway)
					{//If the condition decrements.
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
				if(ifsRequired)
				{
					pIRTokensVec[ti]=new IRTokenIfOpen();
				}else
				{
					pIRTokensVec[ti]=new IRTokenNoOp(pIRTokensVec[ti]);
				}
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
					if(ifsRequired)
					{
						IRTokenIfClose *closingIfBracket =new IRTokenIfClose(false);
						closingIfBracket->offsetCells(pirTokenLoopOpen->cellsAway);
						pIRTokensVec[w]=closingIfBracket;
					}else
					{
						pIRTokensVec[w]=new IRTokenNoOp(pIRTokensVec[w]);
					}
				}else
				{
					pIRTokensVec[noOpR]=new IRTokenNoOp(pIRTokensVec[noOpR]);
					if(ifsRequired)
					{
						IRTokenIfClose *closingIfBracket =new IRTokenIfClose(true);
						closingIfBracket->offsetCells(pirTokenLoopOpen->cellsAway);
						pIRTokensVec[w]=closingIfBracket;
					}else
					{
						pIRTokensVec[w]=new IRTokenNoOp(pIRTokensVec[w]);
					}
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
			std::cout<<"Learned thing in irClear#"<<irClear->getIRTokenID()<<" for ["<<irClear->cellsAway + curRelIndex<<"="<<irClear->setVal<<"]\n";
		}
		IRTokenMultiShift *mshift = dynamic_cast<IRTokenMultiShift*>(irToken);
		if(mshift!=nullptr)
		{
			curRelIndex+=mshift->numShifts;
			//env.knownCells.clear();//Forget everything.
		}
		IRTokenLoopOpen *irLoopOpen = dynamic_cast<IRTokenLoopOpen*>(irToken);
		IRTokenLoopClose *irLoopClose = dynamic_cast<IRTokenLoopClose*>(irToken);
		if(irLoopOpen!=nullptr)//Check for loop condition for dead-code removal
		{
			bool isDeadCode=false;
			try
			{
				int knownCondition = env.knownCells.at(curRelIndex+irLoopOpen->cellsAway);
				if(knownCondition==0)
				{
					isDeadCode=true;
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
				}
			}catch(std::out_of_range& oor){}
			if(!isDeadCode)
			{
				std::cout<<"Forgetting things in irLoopOpen#"<<irLoopOpen->getIRTokenID()<<"...\n";
				//std::cout<<"Forgot everything: loopOpen#"<<irLoopOpen->getIRTokenID()<<"\n";
				
				//Forget all cells modified in scope
				int scope=0;
				for(unsigned int l=ti+1;l<pIRTokensVec.size();l++)
				{
					IRToken *irTokenInLoop = pIRTokensVec[l];
					
					scope+=irTokenInLoop->getScope();
					if(scope<0)break;
					auto *irTokenInLoopMultiAdd = dynamic_cast<IRTokenMultiAdd*>(irTokenInLoop);
					auto *irTokenInLoopMultiShift = dynamic_cast<IRTokenMultiShift*>(irTokenInLoop);
					auto *irTokenInLoopClear = dynamic_cast<IRTokenClear*>(irTokenInLoop);
					auto *irTokenInLoopInput = dynamic_cast<IRTokenInput*>(irTokenInLoop);
					auto *irTokenInLoopMultiply = dynamic_cast<IRTokenMultiply*>(irTokenInLoop);
					if(irTokenInLoopMultiShift!=nullptr)
					{
						env.knownCells.clear();//Forget everything.
						
						std::cout<<"\tForgot everything in irTokenInLoopMultiShift#"<<irTokenInLoopMultiShift->getIRTokenID()<<".\n";
						break;
					}
					if(irTokenInLoopMultiAdd!=nullptr)
					{
						env.knownCells.erase(curRelIndex+irTokenInLoopMultiAdd->cellsAway);
						std::cout<<"\tForgot ["<<(curRelIndex+irTokenInLoopMultiAdd->cellsAway)<<"] in irTokenInLoopMultiAdd#"<<irTokenInLoopMultiAdd->getIRTokenID()<<".\n";
					}
					if(irTokenInLoopClear!=nullptr)
					{
						try
						{
							int cellIndex=curRelIndex+irTokenInLoopClear->cellsAway;
							if(irTokenInLoopClear->setVal!=env.knownCells.at(cellIndex))
							{
								env.knownCells.erase(cellIndex);
								std::cout<<"\tForgot ["<<(cellIndex)<<"] in irTokenInLoopClear#"<<irTokenInLoopClear->getIRTokenID()<<".\n";	
							}
						}catch(std::out_of_range& oor){}					
					}
					if(irTokenInLoopInput!=nullptr)
					{
						env.knownCells.erase(curRelIndex+irTokenInLoopInput->cellsAway);
						std::cout<<"\tForgot ["<<(curRelIndex+irTokenInLoopInput->cellsAway)<<"] in irTokenInLoopInput#"<<irTokenInLoopInput->getIRTokenID()<<".\n";
					}
					if(irTokenInLoopMultiply!=nullptr)
					{
						env.knownCells.erase(curRelIndex+irTokenInLoopMultiply->cellsAway);
						std::cout<<"\tForgot ["<<(curRelIndex+irTokenInLoopMultiply->cellsAway)<<"] in irTokenInLoopMultiply#"<<irTokenInLoopMultiply->getIRTokenID()<<".\n";
					}
					
				}
				curRelIndex=0;//Make known cells relative to current position.
			}
		}
		if(irLoopClose!=nullptr)//In or exiting a loop.
		{
			//std::cout<<"Forgot everything: loopClose#"<<irLoopClose->getIRTokenID()<<"\n";
			env.knownCells.clear();//Forget everything.
						
			//env.knownCells[curRelIndex]=0;//Current must be zero to exit loop
			curRelIndex=0;//Make known cells relative to current position.
		}
		IRTokenInput *irInput = dynamic_cast<IRTokenInput*>(irToken);
		if(irInput!=nullptr)//Cannot know user input.
		{
			env.knownCells.erase(curRelIndex+irInput->cellsAway);
			//std::cout<<"Cleared knownCells.\n";
		}
		IRTokenMultiply *irMult = dynamic_cast<IRTokenMultiply*>(irToken);
		if(irMult!=nullptr)
		{
			try //If factors are known, reduce to an add.
			{
				
				const int knownFactor = env.knownCells.at(curRelIndex+irMult->factorACellsAway);
				const int result = knownFactor * irMult->factor;
				pIRTokensVec[ti] = new IRTokenMultiAdd(result,irMult->cellsAway);
				ti--;continue;//Replaced token, so track it
				
				
			}catch(std::out_of_range& oor)
			{
				//If unknown factors, mark current cell as unknown.
				env.knownCells.erase(curRelIndex+irMult->cellsAway);
			}
		}
		
		IRTokenIfOpen *irIfOpen = dynamic_cast<IRTokenIfOpen*>(irToken);
		IRTokenIfClose *irIfClose = dynamic_cast<IRTokenIfClose*>(irToken);
		if(irIfOpen!=nullptr)
		{
			try
			{
				//std::cout<<"Checking irIfOpen#"<<irIfOpen->getIRTokenID()<<" @ "<<std::to_string(curRelIndex+irIfOpen->cellsAway)<<"\n";
				//std::cout<<"\t"<<env.knownCellsToString()<<"\n";
				int knownCondition = env.knownCells.at(curRelIndex+irIfOpen->cellsAway);
				if(knownCondition==0)//Known false, so remove dead code.
				{
					//std::cout<<"\tKnown false\n";
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
					//std::cout<<"\tKnown true\n";
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
				}
			}catch(std::out_of_range& oor)
			{
				//std::cout<<"\tUnknown value\n";
			}
		}
		if(irIfClose!=nullptr)
		{
			//Condition unknown, so must forget changes.
			env.knownCells.clear();//Forget everything.
			curRelIndex=0;//Make known cells relative to current position.
		}
		IRTokenPrintChar *irPrintChar = dynamic_cast<IRTokenPrintChar*>(irToken);
		if(irPrintChar!=nullptr)
		{
			try{
				if(!irPrintChar->hasKnownCharValue()){
					int val = env.knownCells.at(curRelIndex+irPrintChar->cellsAway);
					irPrintChar->knownCharValue=val;	
				}
			}catch(std::out_of_range& oor){}
		}
	}
}
void optimizeIRTokensCancelShifts(std::vector<IRToken*>& pIRTokensVec)
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
void optimizeIRTokensDiffuseShifts(std::vector<IRToken*>& pIRTokensVec){
	for(unsigned int ti=0;ti<pIRTokensVec.size();ti++)
	{
		IRToken *irToken = pIRTokensVec[ti];
		IRTokenMultiShift *irShift = dynamic_cast<IRTokenMultiShift*>(irToken);
		if(irShift!=nullptr)
		{
			IRTokenMultiShift *otherShift = nullptr;
			int scope=0;
			unsigned int latestPos=ti;
			for(unsigned int s=ti+1;s<pIRTokensVec.size();s++)
			{
				IRToken *irTokenS = pIRTokensVec[s];
				scope+=irTokenS->getScope();
				if(scope<0)
				{
					break;//Out of scope
				}
				latestPos=s;
				if(scope==0 && otherShift==nullptr)
				{
					otherShift=dynamic_cast<IRTokenMultiShift*>(irTokenS);
					break;//Found next shift in same scope
				}
			}
			if(latestPos>ti)
			{
				pIRTokensVec[ti] = new IRTokenNoOp(pIRTokensVec[ti]);
				for(unsigned int o=ti+1;o<=latestPos;o++)
				{
					IRToken *irTokenO = pIRTokensVec[o];
					irTokenO->offsetCells(irShift->numShifts);
				}
				if(otherShift==nullptr){
					pIRTokensVec.insert(pIRTokensVec.begin()+latestPos+1,irShift);
				}else
				{
					otherShift->numShifts+=irShift->numShifts;
				}
			}
		}
	}
}
void optimizeIRTokensReduceMultiplyIfs(std::vector<IRToken*>& pIRTokensVec)
{
	//if an IF scope only contains multiplies using the condition
	//and a clear for the condition to zero, then the if is not required
	for(unsigned int ti=0;ti<pIRTokensVec.size();ti++)
	{
		IRToken *irToken = pIRTokensVec[ti];
		IRTokenIfOpen *ifToken = dynamic_cast<IRTokenIfOpen*>(irToken);
		if(ifToken!=nullptr)
		{
			unsigned int s=ti+1;
			int scope=0;
			bool reqIfs=false;
			for(;s<pIRTokensVec.size();s++)
			{
				IRToken *irTokenS = pIRTokensVec[s];
				scope+=irTokenS->getScope();
				auto *irTokenClear = dynamic_cast<IRTokenClear*>(irTokenS);
				auto *irTokenMult = dynamic_cast<IRTokenMultiply*>(irTokenS);
				auto *irTokenIfClose = dynamic_cast<IRTokenIfClose*>(irTokenS);
				
				if(irTokenIfClose!=nullptr){
					break;
				}
				if(scope!=0){
					reqIfs=true;break;
				}
				if(irTokenClear!=nullptr)
				{
					if(irTokenClear->cellsAway!=ifToken->cellsAway && irTokenClear->setVal==0){
						reqIfs=true;break;
					}
				}else{
					if(irTokenMult!=nullptr){
						if(irTokenMult->factorACellsAway!=ifToken->cellsAway){
							reqIfs=true;break;
						}
					}else{
						reqIfs=true;break;//Other irToken, may require ifs
					}
				}
			}
			if(!reqIfs)
			{
				pIRTokensVec[ti]=new IRTokenNoOp(pIRTokensVec[ti]);
				pIRTokensVec[s]=new IRTokenNoOp(pIRTokensVec[s]);
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
	optimizeIRTokensCancelShifts(pIRTokensVec);
	optimizeIRTokensDiffuseShifts(pIRTokensVec);
	
	optimizeIRTokensReduceMultiplyIfs(pIRTokensVec);
	optimizeIRTokensKnownVals(pIRTokensVec);
	ridOfNoOps(pIRTokensVec);
	
}
