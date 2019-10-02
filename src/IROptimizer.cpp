#include "IR.hpp"
#include "Environment.hpp"
#include <set>

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
					if(irTokenInLoopMultiShift!=nullptr){
						env.knownCells.clear();//Forget everything.
						break;
					}
					if(irTokenInLoopMultiAdd!=nullptr){
						env.knownCells.erase(curRelIndex+irTokenInLoopMultiAdd->cellsAway);
					}
					if(irTokenInLoopClear!=nullptr){
						try{
							int cellIndex=curRelIndex+irTokenInLoopClear->cellsAway;
							if(irTokenInLoopClear->setVal!=env.knownCells.at(cellIndex)){
								env.knownCells.erase(cellIndex);
							}
						}catch(std::out_of_range& oor){}					
					}
					if(irTokenInLoopInput!=nullptr){
						env.knownCells.erase(curRelIndex+irTokenInLoopInput->cellsAway);
					}
					if(irTokenInLoopMultiply!=nullptr){
						env.knownCells.erase(curRelIndex+irTokenInLoopMultiply->cellsAway);
					}
					
				}
				if(env.knownCells.empty())
				{
					curRelIndex=0;//Make known cells relative to current position.
				}
			}
		}
		if(irLoopClose!=nullptr)//In or exiting a loop.
		{
			env.knownCells.clear();
			curRelIndex=0;//Make known cells relative to current position.
		}
		IRTokenInput *irInput = dynamic_cast<IRTokenInput*>(irToken);
		if(irInput!=nullptr)//Cannot know user input.
		{
			env.knownCells.erase(curRelIndex+irInput->cellsAway);
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
				int knownCondition = env.knownCells.at(curRelIndex+irIfOpen->cellsAway);
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
				}
			}catch(std::out_of_range& oor){}
		}
		if(irIfClose!=nullptr)
		{
			//If this token exists, the entire if statement is known not to be dead code.
			//So we search up to the matching ifOpen, making sure to forget all set cells.
			int scope=0;
			for(unsigned int l=ti-1;l>0;l--)
			{
				IRToken *irTokenInIf = pIRTokensVec[l];
					
				scope+=irTokenInIf->getScope();
				if(scope>0)break;
				auto *irTokenInIfMultiAdd = dynamic_cast<IRTokenMultiAdd*>(irTokenInIf);
				auto *irTokenInIfMultiShift = dynamic_cast<IRTokenMultiShift*>(irTokenInIf);
				auto *irTokenInIfClear = dynamic_cast<IRTokenClear*>(irTokenInIf);
				auto *irTokenInIfInput = dynamic_cast<IRTokenInput*>(irTokenInIf);
				auto *irTokenInIfMultiply = dynamic_cast<IRTokenMultiply*>(irTokenInIf);
				auto *irTokenInIfCloseIf = dynamic_cast<IRTokenIfClose*>(irTokenInIf);
				if(irTokenInIfCloseIf!=nullptr){
					if(irTokenInIfCloseIf->doesClear)
						env.knownCells.erase(curRelIndex+irTokenInIfCloseIf->cellsAway);
				}
				if(irTokenInIfMultiShift!=nullptr){
					env.knownCells.clear();//Forget everything.
					break;
				}
				if(irTokenInIfMultiAdd!=nullptr){
					env.knownCells.erase(curRelIndex+irTokenInIfMultiAdd->cellsAway);
					continue;
				}
				if(irTokenInIfMultiply!=nullptr){
					env.knownCells.erase(curRelIndex+irTokenInIfMultiply->cellsAway);
					continue;
				}
				if(irTokenInIfClear!=nullptr){
					try{
						int cellIndex=curRelIndex+irTokenInIfClear->cellsAway;
						env.knownCells.erase(cellIndex);
						continue;
					}catch(std::out_of_range& oor){}					
				}
				if(irTokenInIfInput!=nullptr){
					env.knownCells.erase(curRelIndex+irTokenInIfInput->cellsAway);
					continue;
				}
			}
			//env.knownCells.clear();//Forget everything.
			if(env.knownCells.empty()){
				curRelIndex=0;//Make known cells relative to current position.
			}
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

void optimizeIRTokensCondenseSets(std::vector<IRToken*>& pIRTokensVec)
{
	for(unsigned int ti=0;ti<pIRTokensVec.size();ti++)
	{
		IRToken *pIRToken = pIRTokensVec[ti];
		IRTokenMultiAdd *madd = dynamic_cast<IRTokenMultiAdd*>(pIRToken);
		IRTokenClear *clear = dynamic_cast<IRTokenClear*>(pIRToken);
		if(madd!=nullptr || clear!=nullptr)
		{
			int cell =0;
			if(madd!=nullptr)
			{
				cell=madd->cellsAway;
			}
			if(clear!=nullptr)
			{
				cell=clear->cellsAway;
			}
			int scope=0;
			for(unsigned int s=ti+1;s<pIRTokensVec.size();s++)
			{
				IRToken *pIRTokenS = pIRTokensVec[s];
				scope+=pIRTokenS->getScope();
				if(scope!=0)break;
				
				IRTokenMultiAdd *maddS = dynamic_cast<IRTokenMultiAdd*>(pIRTokenS);
				auto *printChar = dynamic_cast<IRTokenPrintChar*>(pIRTokenS);
				auto *input = dynamic_cast<IRTokenInput*>(pIRTokenS);
				auto *shift = dynamic_cast<IRTokenMultiShift*>(pIRTokenS);
				auto *mult = dynamic_cast<IRTokenMultiply*>(pIRTokenS);
				if(shift!=nullptr){break;}
				if(mult!=nullptr)
				{
					if(mult->factorACellsAway==cell)break;
				}
				if(printChar!=nullptr)
				{
					if(!printChar->hasKnownCharValue())
					{
						if(madd!=nullptr&&printChar->cellsAway==madd->cellsAway){break;}
						if(clear!=nullptr&&printChar->cellsAway==clear->cellsAway){break;}
					}
				}
				if(input!=nullptr)
				{
					if(input->cellsAway==cell)
					{
						pIRTokensVec[ti]= new IRTokenNoOp(pIRTokensVec[ti]);
						break;
					}
				}
				if(maddS!=nullptr)
				{
					if(madd!=nullptr)
					{
						if(maddS->cellsAway==madd->cellsAway)
						{
							maddS->intVal+=madd->intVal;
							pIRTokensVec[ti]= new IRTokenNoOp(pIRTokensVec[ti]);
							break;
						}
					}
					if(clear!=nullptr)
					{
						if(maddS->cellsAway==clear->cellsAway)
						{
							//maddS->intVal+=clear->setVal;
							clear->setVal+=maddS->intVal;
							pIRTokensVec[ti]= new IRTokenNoOp(pIRTokensVec[ti]);
							pIRTokensVec[s]=clear;
							break;
						}
					}
				}
				IRTokenClear *clearS = dynamic_cast<IRTokenClear*>(pIRTokenS);
				if(clearS!=nullptr)
				{
					if(clearS->cellsAway==cell)
					{
						pIRTokensVec[ti]= new IRTokenNoOp(pIRTokensVec[ti]);
						break;
					}
				}
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
void optimizeIRTokensSurroundShifts(std::vector<IRToken*>& pIRTokensVec)
{
	
	//Take every while loop, and surround with shifts according to the condition
	//so that the index addition only happens twice, not for every conditional check.
	std::map<int,int> shiftsReq;
	std::set<int> opens;
	std::set<int> closes;
	std::vector<int> opensAndCloses;
	//Traverse backwards to maintain order when applying shifts
	for(int ti=pIRTokensVec.size()-1;ti>=0;ti--)
	{
		IRToken *irToken = pIRTokensVec[ti];
		IRTokenLoopClose *irTokenLoopClose = dynamic_cast<IRTokenLoopClose*>(irToken);
		if(irTokenLoopClose!=nullptr)
		{
			int cond = irTokenLoopClose->loopOpen->cellsAway;
			int scope=0;
			int l=ti-1;
			for(;l>=0;l--)
			{
				IRToken *irTokenL = pIRTokensVec[l];
				scope+=irTokenL->getScope();
				irTokenL->offsetCells(-cond);
				if(scope>=1)break;
			}
			shiftsReq[ti]=cond;
			shiftsReq[l]=cond;
			closes.insert(ti);
			opens.insert(l);
			opensAndCloses.push_back(ti);
		}
		
		IRTokenLoopOpen *irTokenLoopOpen = dynamic_cast<IRTokenLoopOpen*>(irToken);
		if(irTokenLoopOpen!=nullptr)
		{
			opensAndCloses.push_back(ti);
		}
	}
	for(unsigned int i=0;i<opensAndCloses.size();i++)
	{
		int irIndex = opensAndCloses[i];
		if(opens.find(irIndex)!=opens.end() &&shiftsReq[irIndex]!=0)
		{
			IRTokenMultiShift *existingShift = dynamic_cast<IRTokenMultiShift*>(pIRTokensVec[irIndex]);
			if(existingShift!=nullptr)
			{
				existingShift->numShifts+=shiftsReq[irIndex];
			}else
			{
				pIRTokensVec.insert(pIRTokensVec.begin()+irIndex,new IRTokenMultiShift(shiftsReq[irIndex]));	
			}
			continue;
		}
		if(closes.find(irIndex)!=closes.end() &&shiftsReq[irIndex]!=0)
		{
			IRTokenMultiShift *existingShift = dynamic_cast<IRTokenMultiShift*>(pIRTokensVec[irIndex+1]);
			if(existingShift!=nullptr)
			{
				existingShift->numShifts-=shiftsReq[irIndex];
			}else
			{
				pIRTokensVec.insert(pIRTokensVec.begin()+irIndex+1,new IRTokenMultiShift(-shiftsReq[irIndex]));
			}
		}
	}
}
void optimizeIRTokens(std::vector<IRToken*>& pIRTokensVec)
{
	int numSweeps=2;
	for(int i=0;i<numSweeps;i++)
	{
		optimizeIRTokensMultiplyPass(pIRTokensVec);
		optimizeIRTokensCancelShifts(pIRTokensVec);
		optimizeIRTokensDiffuseShifts(pIRTokensVec);
		
		optimizeIRTokensReduceMultiplyIfs(pIRTokensVec);
		optimizeIRTokensComplexLoops(pIRTokensVec);

		optimizeIRTokensKnownVals(pIRTokensVec);
		
		optimizeIRTokensCondenseSets(pIRTokensVec);
		ridOfNoOps(pIRTokensVec);
		
		
	}
	optimizeIRTokensSurroundShifts(pIRTokensVec);
	optimizeIRTokensCombineAddsMults(pIRTokensVec);
	ridOfNoOps(pIRTokensVec);
	optimizeIRTokensKnownVals(pIRTokensVec);
}
