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

void FrontEndInterface::print() {
	for (vector<IRNode*>::iterator it = ir_list.begin(); it != ir_list.end();it++) {
		if ((*it)->label != NULL) {
			std::cout << "L" << (*it)->label->index << ": ";
		}
		(*it)->print();
	}
}
void FrontEndInterface::print_for_json() {
	std::cout << "\"BIR\":";
	std::cout << "[" << std::endl;
	for (vector<IRNode*>::iterator it = ir_list.begin(); it != ir_list.end(); it++) {
		IRNode *printNode = (*it);

		std::cout << "{" << std::endl;
		std::cout << "\"hasLabel\":";
		if (printNode->label != NULL) {
			std::cout << "true," << std::endl;
			std::cout << "\"label\":" << printNode->label->index << "," << std::endl;
		}
		else {
			std::cout << "false," << std::endl;
		}
		std::cout << "\"code\":\"";
		printNode->print_for_json();
		std::cout << "\"" << std::endl;
		if (printNode->next != NULL)std::cout << "}," << std::endl;
		else std::cout << "}" << std::endl;

	}
	std::cout << "]," << std::endl;
}