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

		LabelNode *res = label_find(printNode);
		if (res != NULL) {
			std::cout << "L" << res->index << ": ";
		}

		printNode->print();
		printNode = printNode->next;
	}
}

// private
void IRCreator::addIRNode(IRNode *node) {
	std::cout << "IR: ";
	node->print();
	node->next = NULL;
	if (head == NULL) {
		head = node;
		cur = head;
	}
	else {
		cur->next = node;
		node->front = cur;
		cur = node;
	}

	if (is_parse_if) {
		_expect_true_label->target = node;
		label_finish(_expect_true_label);
		is_parse_if = false;
	}

	if (is_parse_else) {
		_expect_false_label->target = node;
		label_finish(_expect_false_label);
		//std::cout << "!!!!!!!!!!!!!!!!" << std::endl;
		is_parse_else = false;
	}

	if (is_parse_if_end) {

		_expect_end_label->target = node;
		label_finish(_expect_end_label);
		_expect_end_label = NULL;
		is_parse_if_end = false;
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

bool IRCreator::handle_logic_exp() {
	

	return true;
}

bool IRCreator::handle_if_state_1() {
	if (_expect_end_label == NULL) {
		IRNode *ir = new IRNode(IRType::jump);
		LabelNode *label = new LabelNode(NULL, ir);
		ir->setArg(0, label);
		//ss_push(new SSNode(label));
		addIRNode(ir);
		_expect_end_label = label;
	}
	else {
		IRNode *ir = new IRNode(IRType::jump);
		ir->setArg(0, _expect_end_label);
		//ss_push(new SSNode(label));
		addIRNode(ir);
	}

	LabelNode *label = ss_get(sp_top() + 1)->label;
	_expect_false_label = label;

	ss_pop();
	ss_pop();

	is_parse_else = true;

	return true;
}

bool IRCreator::handle_if_state_2() {

	is_parse_if_end = true;
	return true;
}

bool IRCreator::handle_if_state_3() {

	LabelNode *label = ss_get(sp_top())->label;
	_expect_true_label = label;

	is_parse_if = true;
	return true;
}

bool IRCreator::handle_if_state_4() {
	LabelNode *label = ss_get(sp_top() + 1)->label;
	_expect_false_label = label;

	ss_pop();
	ss_pop();

	is_parse_else = true;
	return true;
}

bool IRCreator::handle_logic_or_exp_2() {
	int start_index = sp_top(), fin_index = ss_len();
	LabelNode *true_label = ss_get(start_index)->label, *false_label = ss_get(fin_index - 1)->label;
	for (int i = start_index; i < fin_index; i++)
		ss_pop();
	ss_push(new SSNode(true_label));
	ss_push(new SSNode(false_label));
	return true;
}

bool IRCreator::handle_logic_and_exp_1() {


	return true;
}

bool IRCreator::handle_logic_and_exp_2() {

	int start_index = sp_top(), fin_index = ss_len();

	SSNode *bro_true_node = ss_get(sp_get(1)), *bro_false_node = ss_get(sp_get(1) + 1);
	LabelNode *true_label, *false_label;

	if (bro_true_node != NULL && bro_true_node->type == SSType::label && bro_true_node != ss_get(start_index)) {
		//std::cout << "########################" << std::endl;
		bro_false_node->label->target = ss_get(start_index)->label->from;
		label_finish(bro_false_node->label);
		true_label = bro_true_node->label;
		ss_get(fin_index - 2)->label->from->setArg(2, true_label);
	}
	else {
		true_label = ss_get(fin_index - 2)->label;
	}

	false_label = ss_get(fin_index - 1)->label;

	for (int i = start_index; i < fin_index; i++) {
		ss_pop();
	}

	ss_push(new SSNode(true_label));
	ss_push(new SSNode(false_label));

	return true;
}

bool IRCreator::handle_rel_exp(Token op) {
	int start_index = sp_top(), fin_index = ss_len();
	SSNode *rel_left = ss_get(start_index),*rel_right = ss_get(start_index + 1);

	IRNode *true_jump = new IRNode((IRType)(op - Token::double_equal + (int)IRType::equal_jump), NULL);
	IRNode *false_jump = new IRNode(IRType::jump, NULL);

	addIRNode(true_jump);
	addIRNode(false_jump);

	set_arg(true_jump, 0, rel_left);
	set_arg(true_jump, 1, rel_right);

	ss_pop();
	ss_pop();

	SSNode *bro_true_node = ss_get(sp_get(1)), *bro_false_node = ss_get(sp_get(1) + 1);
	LabelNode *true_label, *false_label;

	true_label = new LabelNode(NULL, true_jump);
	if (bro_true_node != NULL && bro_true_node->type == SSType::label) {
		false_label = bro_false_node->label;
		//
		ss_get(start_index - 2)->label->target = true_jump;
		label_finish(ss_get(start_index - 2)->label);
	}
	else {
		false_label = new LabelNode(NULL, false_jump);
	}
	ss_push(new SSNode(true_label));
	ss_push(new SSNode(false_label));

	true_jump->setArg(2, true_label);
	false_jump->setArg(0, false_label);

	return true;
}

bool IRCreator::handle_func_call() {
	int start_index = sp_top(), fin_index = ss_len();
	int temp = temp_top_index++;
	

	FuncNode *func = stm->findFunc(ss_get(start_index)->string_val);
	if (func == NULL) {
		return false;
	}
	// 判断参数数目
	if (fin_index - start_index - 1 != func->param_num) {
		return false;
	}

	IRNode *ir = new IRNode(IRType::func_call, NULL);
	IRNode *param_in = NULL;
	// 传参
	for (int i = start_index + 1; i < fin_index; i++) {
		param_in = new IRNode(IRType::func_param_in, NULL);
	}
	return true;
}