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
	SymbolTable *whole_table; // ȫ��������ķ��ű�
	std::vector<IRNode *> ir_list;
	int ir_num;
private:
	
};

