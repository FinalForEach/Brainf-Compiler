#pragma once
#include <map>
#include <string>
#include <optional>
class Environment 
{
	public:
	std::map<int,int> knownCells;
	Environment()
	{
		for(int i=-30000;i<30000;i++)
		{
			knownCells[i]=0;//Initialize
		}
	}
	std::string knownCellsToString();
};
