#include "IRCreator.h"
#include<functional>

bool IRCreator::handle_state() {
	int start_index = sp_top(), fin_index = ss_len();
	for (int i = start_index; i < fin_index; i++) {
		ss_pop();

	}
	return true;
}

bool IRCreator::handle_assign_exp() {
	int start_index = sp_top(), fin_index = ss_len();

	if (fin_index - start_index == 1) {
		return true;
	}

	SSNode *assign_right = ss_get(fin_index - 1);
	IRNode *ir = NULL;
	for (int i = start_index; i < fin_index - 1; i++) {
		SSNode *left = ss_get(i);

		if (left->type == SSType::identifier) {
			VarNode *id = NULL;
			_handle_var_undecl(id, left);
			ir = new IRNode(IRType::assign, NULL);
			ir->setArg(1, id);
			_set_arg(ir, 0, assign_right);
			
			addIRNode(ir);
		}

	}

	SSNode *p = new SSNode(assign_right);

	for (int i = start_index; i < fin_index; i++)
		ss_pop();
	ss_push(p);
	return true;

}

bool IRCreator::handle_additive_exp() {

	int start_index = sp_top(), fin_index = ss_len();

	if (fin_index - start_index == 1) {
		//std::cout << "HAE  " << (int)ss_get(start_index)->type << " " << ss_get(start_index)->float_val << " " << ss_get(start_index)->int_val << std::endl;
	}
	else {
		
		float x_const = 0;
		bool has_const = false;
		int first_ti = -1;
		SSNode *first_add = NULL;
		SSNode *node = ss_get(start_index);
		int it_start = (node->type == SSType::sub) ? start_index + 1 : start_index;
		for (int i = it_start; i < fin_index; i++) {
			node = ss_get(i);
			if (node->type == SSType::int_const) {
				if (i == start_index || ss_get(i - 1)->type == SSType::add)
					x_const += node->int_val;
				else
					x_const -= node->int_val;
				has_const = true;
			}
			else if (node->type == SSType::identifier || node->type == SSType::temp_var) {
				if ((i == start_index || ss_get(i - 1)->type == SSType::add) && first_add == NULL)
					first_add = node;
				if (first_ti == -1)
					first_ti = i;
			}
			/*else if (node->type == SSType::temp_var) {
				if (first_ti == -1)
					first_ti = i;
			}*/
		}
		if (first_ti == -1) {// 常数表达式
			
			for (int i = start_index; i < fin_index; i++)
				ss_pop();
			ss_push(new SSNode(SSType::int_const, 0, (int)x_const));
		}
		else {
			SSNode *node = ss_get(first_ti);
			SSNode *a1_node = NULL;
			SSNode *oper = NULL;
			IRNode *ir = NULL;
			IRType type;
			TempNode *temp = new TempNode(temp_top_index++);
			stm->insert(temp);
			int it_start = 0;
			//int temp = temp_top_index++;
			if (!has_const && first_add != NULL) {
				
				if (first_add == node) {
					oper = ss_get(first_ti + 1);
					ir = new IRNode((oper->type == SSType::add) ? IRType::add : IRType::sub, NULL);
					_set_arg(ir, 1, node);
					_set_arg(ir, 0, ss_get(first_ti + 2));

					ir->setArg(2, temp); 
					addIRNode(ir);
					it_start = first_ti + 4;
				}
				else {
					ir = new IRNode(IRType::sub, NULL);
					_set_arg(ir, 0, node);
					_set_arg(ir, 1, first_add);
					//std::cout << "SSSSSSSSSs";
					ir->setArg(2, temp);
					addIRNode(ir);
					it_start = first_ti + 2;
				}
				
			}
			else {
				
				if (first_ti == start_index)
					type = IRType::add;
				else
					type = ss_get(first_ti - 1)->type == SSType::add ? IRType::add : IRType::sub;
				ir = new IRNode(type, NULL);
				
				ir->setArg(1, (int)x_const);
				_set_arg(ir, 0, node);
				//set_arg(ir, 1, node);
				ir->setArg(2, temp);
				addIRNode(ir);
				it_start = first_ti + 2;
			}
			for (int i = it_start; i < fin_index; i += 2) {
				a1_node = ss_get(i);
				if (a1_node == first_add)
					continue;
				else if (a1_node->type != SSType::int_const) {
					type = ss_get(i - 1)->type == SSType::add ? IRType::add : IRType::sub;
					ir = new IRNode(type, NULL);
					_set_arg(ir, 0, a1_node);

					ir->setArg(1, temp);
					ir->setArg(2, temp);
					addIRNode(ir);
				}
			}

			for (int i = start_index; i < fin_index; i++)
				ss_pop();
			ss_push(new SSNode(temp));
		}

	}

	return true;
}

bool IRCreator::handle_multiplicative_exp() {

	int start_index = sp_top(), fin_index = ss_len();
	if (fin_index - start_index == 1) {

	}
	else {
		float x_const = 1;
		bool has_const = false;
		int first_ti = -1;
		SSNode *first_ti_node = NULL;
		for (int i = start_index; i < fin_index; i+=2) {
			SSNode *node = ss_get(i);
			if (node->type == SSType::int_const) {
				if (i == start_index || ss_get(i - 1)->type == SSType::mult)
					x_const *= node->int_val;
				else
					x_const /= node->int_val;
				has_const = true;
			}
			else if (node->type == SSType::identifier) {
				if (first_ti == -1)
					first_ti = i;
			}
			else if (node->type == SSType::temp_var) {
				if (first_ti == -1)
					first_ti = i;
			}
		}
		if (first_ti == -1) {// 常数表达式
			for (int i = start_index; i < fin_index; i++)
				ss_pop();
			ss_push(new SSNode(SSType::int_const, 0, (int)x_const));
		}
		else {
			SSNode *node = ss_get(first_ti);
			SSNode *a1_node = NULL;
			SSNode *oper = NULL;
			IRNode *ir = NULL;
			IRType type;
			TempNode *temp = new TempNode(temp_top_index++);
			stm->insert(temp);
			int it_start = 0;
			//int temp = temp_top_index++;
			if (!has_const) {
				oper = ss_get(first_ti + 1);
				ir = new IRNode((oper->type == SSType::mult)?IRType::mult:IRType::div, NULL);
				_ir_set_arg(set_arg(ir, 1, node));
				_ir_set_arg(set_arg(ir, 0, ss_get(first_ti + 2)));

				ir->setArg(2, temp);
				addIRNode(ir);
				it_start = first_ti + 4;
			}
			else {
				if (first_ti == start_index)
					type = IRType::mult;
				else
					type = ss_get(first_ti - 1)->type == SSType::mult ? IRType::mult : IRType::div;
				ir = new IRNode(type, NULL);
				ir->setArg(1, (int)x_const);
				_ir_set_arg(set_arg(ir, 0, node));
				//set_arg(ir, 1, node);
				ir->setArg(2, temp);
				addIRNode(ir);
				it_start = first_ti + 2;
			}
			for (int i = it_start; i < fin_index; i += 2) {
				a1_node = ss_get(i);
				if (a1_node->type != SSType::int_const) {
					type = ss_get(i - 1)->type == SSType::mult ? IRType::mult : IRType::div;
					ir = new IRNode(type, NULL);
					_ir_set_arg(set_arg(ir, 0, a1_node));
					
					ir->setArg(1, temp);
					ir->setArg(2, temp);
					addIRNode(ir);
				}
			}

			for (int i = start_index; i < fin_index; i++)
				ss_pop();
			ss_push(new SSNode(temp));
		}
	}

	return true;
}

