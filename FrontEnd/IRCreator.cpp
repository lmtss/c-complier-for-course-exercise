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

	FuncNode *func = stm->findFunc(id->string_val);
	if (func == NULL) {
		func = new FuncNode;

		func->retType = stm->getBasicType((int)type_spec->type - 50);
		func->name = id->string_val;

		func->param_num = 0;
		func->table = NULL;
		for (int i = type_spec_index + 2; i < sp_top(); i+=2) {
			VarNode *param = new VarNode;
			SSNode *type_node = ss_get(i);
			param->declPosLine = type_node->code_line;
			param->varType = stm->getBasicType((int)type_node->type - 50);

			SSNode *id_node = ss_get(i + 1);
			param->name = id_node->string_val;
			func->paraList.push_back(param);
			func->param_num++;
		}
		stm->insertFunc(func);
	}
	else {
		// ret type
		if (func->retType != stm->getBasicType((int)type_spec->type - 50)) {

		}
		// param num
		int def_param_num = (sp_top() - type_spec_index - 2) / 2;
		if (def_param_num != func->param_num) {

		}
		// param type
		for (int i = 0; i < def_param_num; i++) {
			TypeNode *decl_param_type = func->paraList[i]->varType;
			SSNode *ts = ss_get(type_spec_index + 2 + 2 * i);
			TypeNode *def_param_type = stm->getBasicType((int)ts->type - 50);

			if (def_param_type != decl_param_type) {

			}

			SSNode *is = ss_get(type_spec_index + 2 + 2 * i + 1);
			func->paraList[i]->name = is->string_val;
			func->paraList[i]->declPosLine = is->code_line;
		}
	}

	
	func->table = stm->addFunc(func);
	for (int i = 0; i < func->param_num; i++) {
		func->table->insert(func->paraList[i]->name, func->paraList[i]);
		func->paraList[i]->level = stm->getCurLevel();
		func->cur_address += func->paraList[i]->varType->len;
	}

	IRNode *ir = new IRNode(IRType::func, NULL);
	ir->setArg(0, func);
	
	addIRNode(ir);

	if (func->retType == stm->getBasicType(2)) {
		//std::cout << "expect return" << std::endl;
		//has_return = false;
		is_void_func = true;
	}
	has_return = false;
}
bool IRCreator::handle_func_def() {
	//std::cout << "adasdas " << ss_len() - sp_top();
	for (int i = sp_top(); i < ss_len() + 1; i++)
		ss_pop();
	if (!has_return && !is_void_func) {
		if (is_void_func) {
			IRNode *ir = new IRNode(IRType::ret, NULL);
			addIRNode(ir);
		}
		else {
			NoRetError *e = new NoRetError(stm->getCurFunc());
			em->addEN(e);
			return false;
		}
		
	}
	FuncNode *func = stm->getCurFunc();
	func->isDefinied = true;
	func->size = func->cur_address + func->max_arg_size + 8;
	return true;
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
	std::cout << "-----------------IR------------------" << std::endl;
	IRNode *printNode = head;
	while (printNode != NULL) {

		LabelNode *res = label_find(printNode);
		if (res != NULL) {
			std::cout << "L" << res->index << ": ";
		}

		printNode->print();
		printNode = printNode->next;
	}
	std::cout << "----------------ERROR----------------" << std::endl;
	em->print();
}

void IRCreator::print_json() {
	//std::cout << "-----------------IR------------------" << std::endl;
	IRNode *printNode = head;
	
	//std::cout << "{" << std::endl; 
	std::cout << "\"IR\":";
	std::cout << "[" << std::endl;
	while (printNode != NULL) {
		std::cout << "{" << std::endl;
		std::cout << "\"hasLabel\":";
		LabelNode *res = label_find(printNode);
		if (res != NULL) {
			std::cout << "true," << std::endl;
			std::cout << "\"label\":" << res->index << "," << std::endl;
		}
		else {
			std::cout << "false," << std::endl;
		}
		std::cout << "\"code\":\"";
		printNode->print_for_json();
		std::cout << "\"" << std::endl;
		if(printNode->next != NULL)std::cout << "}," << std::endl;
		else std::cout << "}" << std::endl;
		
		printNode = printNode->next;
	}
	std::cout << "]," << std::endl;
	if (em->hasError()) {
		std::cout << "\"hasError\":true,"<< std::endl; 
		std::cout << "\"ERROR\":[" << std::endl;
		em->print_for_json();
		std::cout << "]" << std::endl;;
	}
	else {
		std::cout << "\"hasError\":false";
	}
	
	//std::cout << "}";
}

// private
void IRCreator::addIRNode(IRNode *node) {
	//std::cout << "IR: ";
	//node->print();
	node->next = NULL;
	

	if (!expect_for_exp_3) {
		ir_num++;
		if (head == NULL) {
			head = node;
			cur = head;
		}
		else {
			cur->next = node;
			node->front = cur;
			cur = node;
		}
		if (expect_for_rel_exp) {
			rel_exp_first = node;
			expect_for_rel_exp = false;
		}
		if (expect_for_and_exp) {
			and_exp_first = node;
			expect_for_and_exp = false;
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

		if (_expect_while_to_logic_label != NULL) {
			_expect_while_to_logic_label->target = node;
			label_finish(_expect_while_to_logic_label);
			_expect_while_to_logic_label = NULL;
		}

	}
	else {
		if (for_exp_3_head == NULL) {
			for_exp_3_head = node;
			for_exp_3_end = for_exp_3_head;
		}
		else {
			for_exp_3_end->next = node;
			node->front = for_exp_3_end;
			for_exp_3_end = node;
		}
	}

}

bool IRCreator::handle_return_state() {

	SSNode *ret = ss_get(sp_top());
	IRNode *ir = NULL;
	ir = new IRNode(IRType::ret, NULL);
	_ir_set_arg(set_arg(ir, 0, ret))
	//set_arg(ir, 0, ret);
	/*if (ret->type == SSType::identifier) {
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
	}*/

	addIRNode(ir);
	has_return = true;

	return true;
}

bool IRCreator::handle_logic_exp() {
	

	return true;
}

bool IRCreator::handle_if_state_1() {
	if (_expect_end_label == NULL) {
		IRNode *ir = new IRNode(IRType::jump, NULL);
		LabelNode *label = new LabelNode(NULL, ir);
		ir->setArg(0, label);
		//ss_push(new SSNode(label));
		addIRNode(ir);
		_expect_end_label = label;
	}
	else {
		IRNode *ir = new IRNode(IRType::jump, NULL);
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
		//bro_false_node->label->target = ss_get(start_index)->label->from;
		bro_false_node->label->target = and_exp_first;
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

	_set_arg(true_jump, 0, rel_left);
	_set_arg(true_jump, 1, rel_right);

	ss_pop();
	ss_pop();

	SSNode *bro_true_node = ss_get(sp_get(1)), *bro_false_node = ss_get(sp_get(1) + 1);
	LabelNode *true_label, *false_label;

	true_label = new LabelNode(NULL, true_jump);
	if (bro_true_node != NULL && bro_true_node->type == SSType::label) {
		false_label = bro_false_node->label;
		//
		ss_get(start_index - 2)->label->target = rel_exp_first;
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
	
	

	FuncNode *func = stm->findFunc(ss_get(start_index)->string_val);

	if (func == NULL) {
		return false;
	}
	// 判断参数数目
	if (fin_index - start_index - 1 != func->param_num) {
		ParamNumError *e = new ParamNumError(ss_get(start_index)->code_line, ss_get(start_index)->string_val, fin_index - start_index - 1, func->param_num);
		em->addEN(e);
		return false;
	}

	IRNode *ir = new IRNode(IRType::func_call, NULL);
	ir->setArg(0, func);
	
	
	
	addIRNode(ir);
	IRNode *param_in = NULL;
	// 传参
	for (int i = start_index + 1; i < fin_index; i++) {
		param_in = new IRNode(IRType::func_param_in, NULL);
		_set_arg(param_in, 0, ss_get(i));
		addIRNode(param_in);
	}
	for (int i = start_index; i < fin_index; i++) {
		ss_pop();
	}
	// void ?
	if (func->retType != stm->getBasicType(2)) {
		TempNode *temp = new TempNode(temp_top_index++);
		stm->insert(temp);
		//int temp = temp_top_index++;
		ir->setArg(1, temp);
		ss_push(new SSNode(temp));
	}

	FuncNode *caller_func = stm->getCurFunc();
	if (caller_func->max_arg_size < func->param_num * 4) {
		caller_func->max_arg_size = func->param_num * 4;
	}

	return true;
}

bool IRCreator::handle_while_state() {
	int start_index = sp_top();
	IRNode *fin_jump = new IRNode(IRType::jump, NULL);
	LabelNode *to_logic_label = ss_get(start_index)->label;
	fin_jump->setArg(0, to_logic_label);
	addIRNode(fin_jump);

	LabelNode *break_label = ss_get(start_index + 2)->label;
	_expect_false_label = break_label;

	ss_pop();
	ss_pop();
	ss_pop();

	is_parse_else = true;

	return true;
}

bool IRCreator::handle_while_state_1() {
	LabelNode *to_logic_label = new LabelNode(NULL, NULL);
	_expect_while_to_logic_label = to_logic_label;
	ss_push(new SSNode(to_logic_label));

	return true;
}

bool IRCreator::handle_while_state_2() {
	LabelNode *label = ss_get(sp_top() + 1)->label;
	_expect_true_label = label;

	is_parse_if = true;
	return true;
}

bool IRCreator::handle_for_state_1() {
	expect_for_exp_3 = true;
	return true;
}

bool IRCreator::handle_for_state_2() {
	expect_for_exp_3 = false;
	return true;
}

bool IRCreator::handle_for_state_3() {
	IRNode *e3_head = for_exp_3_head, *e3_cur = e3_head, *e3_t;
	for_exp_3_head = for_exp_3_end = NULL;
	for (; e3_cur != NULL; e3_cur = e3_t) {
		e3_t = e3_cur->next;
		addIRNode(e3_cur);
	}
	return true;
}

bool IRCreator::handle_print_state() {
	int start_index = sp_top(), fin_index = ss_len();
	for (int i = start_index; i < fin_index; i++) {
		IRNode *ir = new IRNode(IRType::print, NULL);
		SSNode *node = ss_get(i);
		_set_arg(ir, 0, node)
		addIRNode(ir);
	}
	for (int i = start_index; i < fin_index; i++) {
		ss_pop();
	}
	return true;
}