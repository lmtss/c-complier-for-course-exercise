#include "IRCreator.h"


IRCreator::IRCreator() : ss_stack(200, NULL), ss_sp_stack(200, NULL) {
	head = NULL;
	cur = NULL;
	temp_top_index = 0;
	em = new ErrorManager();
}

IRCreator::~IRCreator() {
}

void IRCreator::setSTM(STManager *s) {
	stm = s;
}



void IRCreator::expState() {
	//handleExpression(expStateNode->children[0]);
}

void IRCreator::funcExpect() {

	int type_spec_index = sp_get(1);
	SSNode *type_spec = ss_get(type_spec_index), *id = ss_get(type_spec_index + 1);

	FuncNode *func = new FuncNode;

	func->retType = stm->getBasicType((int)type_spec->type - 50);
	func->name = id->string_val;
	stm->addFunc(func);

	IRNode *ir = new IRNode(IRType::func, func);
	
	addIRNode(ir);
}
void IRCreator::handle_func_def() {
	//std::cout << "adasdas " << ss_len() - sp_top();
	for (int i = sp_top(); i < ss_len() + 1; i++)
		ss_pop();
	
}
void IRCreator::meetRCB() {
	BlockType type = stm->getCurBlockType();

	if (type == BlockType::func_block) {
		/*RetIR *ir = new RetIR;
		ir->isTemp = false;
		ir->varIndex = -1;

		addIRNode(ir);*/
	}

	stm->exitScope();
}

void IRCreator::print() {
	IRNode *printNode = head;
	while (printNode != NULL) {
		printNode->print();
		printNode = printNode->next;
	}
}

// private
void IRCreator::addIRNode(IRNode *node) {
	std::cout << "IR: ";
	node->print();
	if (head == NULL) {
		head = node;
		cur = head;
	}
	else {
		cur->next = node;
		node->front = cur;
		cur = node;
	}
}

bool IRCreator::handle_return_state() {

	SSNode *ret = ss_get(sp_top());
	IRNode *ir = NULL;
	if (ret->type == SSType::identifier) {
		VarNode *var = NULL;
		_handle_var_undecl(var, ret);

		ir = new IRNode(IRType::ret, IRAType::var, var);
	}
	else if (ret->type == SSType::temp_var) {
		ir = new IRNode(IRType::ret, IRAType::temp, ret->int_val);
	}
	else if (ret->type == SSType::int_const) {
		ir = new IRNode(IRType::ret, IRAType::int_imm, ret->int_val);
	}
	else if (ret->type == SSType::float_const) {
		ir = new IRNode(IRType::ret, IRAType::float_imm, ret->float_val);
	}

	addIRNode(ir);

	return true;
}