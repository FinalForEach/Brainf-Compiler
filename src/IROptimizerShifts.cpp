#include "IR.hpp"


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
	//Cleanup "dead" shifts.
	for(unsigned int ti=0;ti<pIRTokensVec.size();ti++)
	{
		IRTokenMultiShift *irShift = dynamic_cast<IRTokenMultiShift*>(pIRTokensVec[ti]);
		if(irShift!=nullptr)
		{
			if(irShift->numShifts==0)
			{//If there is no net shift, just remove it.
				pIRTokensVec[ti] = new IRTokenNoOp(pIRTokensVec[ti]);
			}
		}
	}
}
