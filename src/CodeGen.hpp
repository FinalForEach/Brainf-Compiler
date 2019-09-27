#pragma once
#include <string>
#include "IR.hpp"

std::string generateCode(std::vector<IRToken*>& pIRTokensVec, std::string& outputFilename);
