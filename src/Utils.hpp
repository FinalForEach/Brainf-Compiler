#pragma once
#include <sstream>

std::string loadStrFromFile(std::ifstream& inputFile)
{
	std::stringstream strStream;
	strStream << inputFile.rdbuf();
	
	return strStream.str();
}
