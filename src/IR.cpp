#include "IR.hpp"

void convertTokensToIR(std::vector<Token*>& pTokensVec, std::vector<IRToken*>& pIRTokensVec)
{
	Environment env;
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
	optimizeIRTokens(pIRTokensVec, env);
}

void optimizeIRTokensStageA(std::vector<IRToken*>& pIRTokensVec)
{
	std::vector<IRToken*> pIRTokensVecTmp;
	for(int ti=0;ti<pIRTokensVec.size();ti++)
	{
		IRToken *pIRToken = pIRTokensVec[ti];
		
		//Optimize IRTokenClear by forgetting previous adds
		if(pIRTokensVec.size()-ti>=2 && pIRToken->getName() == "IRTokenMultiAdd")
		{
			if(pIRTokensVec[ti+1]->getName() == "IRTokenClear")
			{
				pIRTokensVecTmp.push_back(pIRTokensVec[ti+1]);//Leave only the clear
				ti+=1;//Consume ir tokens
				continue;
			}
		}
		
		//IRTokenMultiply
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
		
		//Optimize IRTokenMultiAdd by combining with balanced shifts
		if(pIRTokensVec.size()-ti>=3 && pIRToken->getName() == "IRTokenMultiShift")
		{
			if(pIRTokensVec[ti+1]->getName() == "IRTokenMultiAdd")
			{
				IRTokenMultiAdd *maddToken = dynamic_cast<IRTokenMultiAdd*>(pIRTokensVec[ti+1]);
				if(pIRTokensVec[ti+2]->getName() == "IRTokenMultiShift")
				{
					IRTokenMultiShift *shiftAway= dynamic_cast<IRTokenMultiShift*>(pIRTokensVec[ti]);
					IRTokenMultiShift *shiftBack= dynamic_cast<IRTokenMultiShift*>(pIRTokensVec[ti+2]);
					if(shiftAway->numShifts == -(shiftBack->numShifts))
					{
						maddToken->cellsAway=shiftAway->numShifts;
						pIRTokensVecTmp.push_back(maddToken);
						ti+=2;//Consume ir tokens
						continue;
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
void optimizeIRTokensStageB(std::vector<IRToken*>& pIRTokensVec, Environment& env)
{
	std::vector<IRToken*> pIRTokensVecTmp;
	
	//Run through each token, taking note of current cell value
	
	for(int ti=0;ti<pIRTokensVec.size();ti++)
	{
		IRToken *pIRToken = pIRTokensVec[ti];
		
		if(env.hasKnownCellValue()){
			if(pIRToken->getName() == "IRTokenMultiAdd"){
				IRTokenMultiAdd *maddToken = dynamic_cast<IRTokenMultiAdd*>(pIRToken);
				if(maddToken->cellsAway==0)
				{
					env.addToKnownCellValue(maddToken->intVal);
				}
				
			}
			if(pIRToken->getName() == "IRTokenMultiShift"){
				env.forgetCellValue();
			}
			if(pIRToken->getName() == "IRTokenInput"){
				env.forgetCellValue();
			}
			if(pIRToken->getName() == "IRTokenPrintChar")
			{
				IRTokenPrintChar *printCToken = dynamic_cast<IRTokenPrintChar*>(pIRToken);
								
				printCToken->knownCharValue=env.getKnownCellValue();
			}
		}
		if(pIRToken->getName() == "IRTokenClear"){
			env.rememberCellValue(0);
		}
		if(pIRToken->getName() == "IRTokenLoopClose"){
			env.rememberCellValue(0);//Can only exit loop if value is 0
		}
		if(pIRToken->getName() == "IRTokenMultiply"){
			env.rememberCellValue(0);//Multiply clears current cell too
		}
		
		
		pIRTokensVecTmp.push_back(pIRToken);
	}
	
	pIRTokensVec.clear();
	for(int i=0; i<pIRTokensVecTmp.size();i++)
	{
		pIRTokensVec.push_back(pIRTokensVecTmp[i]);
	}
}
void optimizeIRTokensStageC(std::vector<IRToken*>& pIRTokensVec)
{
	std::vector<IRToken*> pIRTokensVecTmp;
	
	std::string printStr = "";
	//Move known printChars into their own section for further optimization
	for(int ti=0;ti<pIRTokensVec.size();ti++)
	{
		IRToken *pIRToken = pIRTokensVec[ti];
		
		if(pIRToken->getName() == "IRTokenPrintChar")
		{
			IRTokenPrintChar *printCToken = dynamic_cast<IRTokenPrintChar*>(pIRToken);
			if(printCToken->isLiteralChar())
			{
				//knownPrintChars.push_back(printCToken);
				
				char c = printCToken->knownCharValue.value();
				switch(c)
				{
					case '"':
					case '\'':
					case '\\': //Require escape chars
					printStr+='\\';
					printStr+=c;
					break;
					default:
					printStr+=c;
					break;
				}
				
				continue;//Consume token
			}else
			{
				if(printStr!="")
				{
					pIRTokensVecTmp.push_back(new IRTokenPrintStr(printStr));
					printStr="";
					//knownPrintChars.clear();
					pIRTokensVecTmp.push_back(pIRToken);
					continue;//Consume token
				}
			}
		}
		if(pIRToken->getName() == "IRTokenLoopClose")
		{
			if(printStr!="")
			{
				pIRTokensVecTmp.push_back(new IRTokenPrintStr(printStr));
				printStr="";
			}
			
		}
		
		
		pIRTokensVecTmp.push_back(pIRToken);
	}

	if(printStr!="")//Print remaining printChars if they exist
	{
		pIRTokensVecTmp.push_back(new IRTokenPrintStr(printStr));
		printStr="";
	}
	
	pIRTokensVec.clear();
	for(int i=0; i<pIRTokensVecTmp.size();i++)
	{
		pIRTokensVec.push_back(pIRTokensVecTmp[i]);
	}
}
void optimizeIRTokensStageD(std::vector<IRToken*>& pIRTokensVec)
{
	std::vector<IRToken*> pIRTokensVecTmp;
	
	for(int ti=0;ti<pIRTokensVec.size();ti++)
	{
		IRToken *pIRToken = pIRTokensVec[ti];
		
		/*int sum=0;
		bool doSumContinue=true;
		for(;ti<pIRTokensVec.size() && pIRTokensVec[ti]->getName()=="IRTokenMultiAdd";ti++)
		{
			//std::cout<<"Going through token: "<<pIRTokensVec[ti]->getName()<<"\n";
			IRTokenMultiAdd *multiAddToken = dynamic_cast<IRTokenMultiAdd*>(pIRTokensVec[ti]);
			if(multiAddToken->cellsAway!=0){
				doSumContinue=false;break;
			}
			sum+=multiAddToken->intVal;
		}
		//std::cout<<sum<<"\n";
		if(sum!=0){
			pIRTokensVecTmp.push_back(new IRTokenMultiAdd(sum));
			if(doSumContinue)continue;
		}*/
		int sum;
		while(pIRToken->getName() == "IRTokenMultiAdd")
		{
			IRTokenMultiAdd *multiAddToken = dynamic_cast<IRTokenMultiAdd*>(pIRToken);
			if(multiAddToken->cellsAway==0) //Do not combine different cells
				sum+=multiAddToken->intVal;
			ti++;if(ti>=pIRTokensVec.size())break;
			
			pIRToken = pIRTokensVec[ti];
		}
		
		if(sum!=0)
		{
			pIRTokensVec.push_back(new IRTokenMultiAdd(sum));
			//ti--;//So that a token is not skipped.
			continue;
		}
		
		pIRTokensVecTmp.push_back(pIRToken);
	}
	for(int i=0; i<pIRTokensVecTmp.size();i++)
	{
		pIRTokensVec.push_back(pIRTokensVecTmp[i]);
	}
}
void optimizeIRTokens(std::vector<IRToken*>& pIRTokensVec, Environment& env)
{
	optimizeIRTokensStageA(pIRTokensVec);
	optimizeIRTokensStageB(pIRTokensVec, env);
	optimizeIRTokensStageC(pIRTokensVec);
	//optimizeIRTokensStageD(pIRTokensVec);
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
