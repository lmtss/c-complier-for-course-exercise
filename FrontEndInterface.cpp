#include "FrontEndInterface.h"
#include <iostream>
FrontEndInterface::FrontEndInterface(IRCreator *irc) : ir_list(irc->ir_num) {
	IRNode *ir = irc->getIRHead();
	for (int i = 0; i < irc->ir_num; i++) {
		ir_list[i] = ir;
		ir = ir->next;
	}
	std::cout << irc->ir_num << " " << ir_list.size();
}

FrontEndInterface::~FrontEndInterface() {
}

IRNode *FrontEndInterface::getIR(int i) {
	return ir_list[i];
}