#include "FrontEndInterface.h"
#include <iostream>
FrontEndInterface::FrontEndInterface(IRCreator *i, STManager *stmanager) : ir_list(i->ir_num) {
	irc = i;
	ir_num = irc->ir_num;
	IRNode *ir = irc->getIRHead();
	for (int i = 0; i < irc->ir_num; i++) {
		ir_list[i] = ir;
		ir = ir->next;
	}
	stm = stmanager;
	whole_table = stmanager->getWholeTable();
}

FrontEndInterface::~FrontEndInterface() {
}

IRNode *FrontEndInterface::getIR(int i) {
	return ir_list[i];
}

LabelNode *FrontEndInterface::getLabel(IRNode *ir) {
	return irc->label_find(ir);
}