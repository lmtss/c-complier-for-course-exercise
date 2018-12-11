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
	VarNode *id = NULL, *last_id = NULL;
	SSNode *left = ss_get(fin_index - 2);
	if (left->type == SSType::identifier) {
		_handle_var_undecl(last_id, left);
		ir = new IRNode(IRType::assign, NULL);
		ir->setArg(1, last_id);
		_set_arg(ir, 0, assign_right);

		addIRNode(ir);
	}

	for (int i = fin_index - 3; i >= start_index; i--) {
		left = ss_get(i);

		if (left->type == SSType::identifier) {
			id = NULL;
			_handle_var_undecl(id, left);
			ir = new IRNode(IRType::assign, NULL);
			ir->setArg(1, id);
			ir->setArg(0, last_id);
			last_id = id;
			
			addIRNode(ir);
		}

	}
	;
	SSNode *p = new SSNode(SSType::identifier, 0, last_id->name.c_str());

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
		bool has_zero = false;
		bool has_ti = false;
		int int_const = 1;
		for (int i = start_index; i < fin_index; i += 2) {
			
			SSNode *node = ss_get(i);
			if (node->type == SSType::int_const) {
				
				if (i == start_index || ss_get(i - 1)->type == SSType::mult) {
					int_const = ss_get(i)->int_val;
					for (i += 2; i < fin_index; i += 2) {
						SSNode *node_2 = ss_get(i);
						if (node_2->type == SSType::int_const) {
							if (ss_get(i - 1)->type == SSType::mult)
								int_const *= node_2->int_val;
							else
								int_const /= node_2->int_val;
						}
						else {
							i -= 2;
							has_ti = true;
							
							break;
						}
					}
					if (int_const == 0) {
						has_zero = true;
						break;
					}
				}
				else {
					for (i += 2; i < fin_index; i += 2) {
						SSNode *node_2 = ss_get(i);
						if (node_2->type != SSType::int_const) {
							i -= 2;
							has_ti = true;
							break;
						}
					}
				}
				//if (has_ti)break;
				
			}
			else {
				
				has_ti = true;
				break;
			}
				
		}

		if (has_zero) {
			for (int i = start_index; i < fin_index; i++)
				ss_pop();
			ss_push(new SSNode(SSType::int_const, 0, 0));
		}
		else if (!has_ti) {
			
			for (int i = start_index; i < fin_index; i++)
				ss_pop();
			ss_push(new SSNode(SSType::int_const, 0, int_const));
		}
		else {
			
			int state = 0;
			IRNode *ir = NULL;
			TempNode *temp = new TempNode(temp_top_index++);
			for (int i = start_index; i < fin_index; i += 2) {
				if (state >= 2) {
					SSNode *op = ss_get(i - 1);
					IRType type = op->type == SSType::mult ? IRType::mult : IRType::div;
					ir = new IRNode(type, NULL);
					ir->setArg(1, temp);
					ir->setArg(2, temp);
					addIRNode(ir);
				}
				SSNode *node = ss_get(i);
				if (node->type == SSType::int_const) {
					int int_const = 1;
					if (i == start_index || ss_get(i - 1)->type == SSType::mult) {
						int_const = ss_get(i)->int_val;
						for (i += 2; i < fin_index; i += 2) {
							SSNode *node_2 = ss_get(i);
							if (node_2->type == SSType::int_const) {
								if (ss_get(i - 1)->type == SSType::mult)
									int_const *= node_2->int_val;
								else
									int_const /= node_2->int_val;
							}
							else {
								i -= 2;
								break;
							}
						}
						if (state == 0) {
							SSNode *op = ss_get(i + 1);
							IRType type = op->type == SSType::mult ? IRType::mult : IRType::div;
							ir = new IRNode(type, NULL);
							ir->setArg(1, int_const);
							ir->setArg(2, temp);
							addIRNode(ir);
						}
						else {
							ir->setArg(0, int_const);
						}
					}
					else {
						for (; i < fin_index; i += 2) {
							SSNode *node_2 = ss_get(i);
							if (node_2->type == SSType::int_const)
								int_const *= node_2->int_val;
							else {
								i -= 2;
								break;
							}
								
						}
						ir->setArg(0, int_const);
					}
					
				}
				else {
					if (state == 0) {
						SSNode *op = ss_get(i + 1);
						IRType type = op->type == SSType::mult ? IRType::mult : IRType::div;
						ir = new IRNode(type, NULL);
						_set_arg(ir, 1, node);
						ir->setArg(2, temp);
						addIRNode(ir);
					}
					else {
						_set_arg(ir, 0, node);
					}
				}
				
				state++;
			}
			for (int i = start_index; i < fin_index; i++)
				ss_pop();
			ss_push(new SSNode(temp));
		}
	}

	return true;
}

