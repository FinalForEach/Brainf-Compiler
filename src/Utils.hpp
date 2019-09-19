#pragma once
#include <sstream>
#include <vector>

std::string loadStrFromFile(std::ifstream& inputFile)
{
	std::stringstream strStream;
	strStream << inputFile.rdbuf();
	
	return strStream.str();
}

void freePointerVector(std::vector<void*> v)
{
	for(int i=0;i<v.size();i++)
	{
		delete v[i];
	}
}
