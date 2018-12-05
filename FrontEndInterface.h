#pragma once
#include "FrontEnd/IRCreator.h"
#include "FrontEnd/SymbolTable.h"
#include <vector>
class FrontEndInterface {
public:
	FrontEndInterface(IRCreator *irc);
	~FrontEndInterface();

	IRNode *getIR(int i);
	
	
	std::vector<IRNode *> ir_list;
private:
	
};

