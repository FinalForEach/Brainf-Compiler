#include "Environment.hpp"
std::string Environment::knownCellsToString()
{
	std::string str="knownCells(size:"+std::to_string(knownCells.size())+ ")=[";
	for(auto& p : knownCells)
	{
		if(p.second!=0 || knownCells.size()<10)
		{
			str+=std::to_string(p.first)+":"+std::to_string(p.second)+", ";
		}
	}
	str+="]\n";
	return str;
}
