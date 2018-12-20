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

void FrontEndInterface::adjust() {
	IRNode *ir = NULL;
	for (vector<IRNode*>::iterator it = ir_list.begin(); it != ir_list.end(); it++) {
		ir = (*it);
		if (ir->type >= IRType::add && ir->type <= IRType::div) {
			if (ir->args[0].type == IRAType::int_imm && ir->args[1].type == IRAType::int_imm) {
				int imm = 0, a1 = ir->args[0].int_imm, a0 = ir->args[1].int_imm;

				ir->args[1] = ir->args[2];
				ir->args[2].type = IRAType::NONE;
				
				switch (ir->type) {
				case IRType::add:imm = a0 + a1; break;
				case IRType::sub:imm = a0 - a1; break;
				case IRType::mult:imm = a0 * a1; break;
				case IRType::div:imm = a0 / a1; break;
				default:
					break;
				}
				ir->type = IRType::assign;
				ir->args[0].int_imm = imm;
			}
		}
	}
}

void FrontEndInterface::print() {
	std::cout << "---------------OPTIMIZE--------------" << std::endl;
	for (vector<IRNode*>::iterator it = ir_list.begin(); it != ir_list.end();it++) {
		if ((*it)->label != NULL) {
			std::cout << "L" << (*it)->label->index << ": ";
		}
		(*it)->print();
	}
}
void FrontEndInterface::print_for_json() {
	std::cout << "\n,\"BIR\":";
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
	std::cout << "]" << std::endl;
}