#pragma once
#include "FrontEnd/IRCreator.h"
#include "FrontEnd/SymbolTable.h"
#include <vector>
class FrontEndInterface {
public:
	FrontEndInterface(IRCreator *i, STManager *stmanager);
	~FrontEndInterface();

	LabelNode *getLabel(IRNode *ir);
	IRNode *getIR(int i);
	STManager *stm;
	IRCreator *irc;
	SymbolTable *whole_table; // 全局作用域的符号表
	std::vector<IRNode *> ir_list;
	int ir_num;
private:
	
};

