#include "IRCreator.h"
#include<functional>



bool IRCreator::handle_assign_exp() {
	int start_index = sp_top(), fin_index = ss_len();

	if (fin_index - start_index == 1) {
		return true;
	}

	SSNode *assign_right = ss_get(fin_index - 1);

	std::function<IRNode*(VarNode *, SSNode *)> func;

	if (assign_right->type == SSType::identifier) {
		/*VarNode *var_right = stm->find(assign_right->string_val);
		if (var_right == NULL) {
			FuncNode *f = stm->getCurFunc();
			int l = assign_right->code_line;
			return false;
		}*/
		VarNode *var_right = NULL;
		_handle_var_undecl(var_right, assign_right)
		func = [var_right](VarNode *var_left, SSNode *exp) {
			IRNode *ir = NULL;
			if (exp == NULL)
				ir = new IRNode(IRType::assign, IRAType::var, var_right, IRAType::var, var_left);
			return ir;
		};
	}
	else if (assign_right->type == SSType::temp_var) {
		int temp_right = assign_right->int_val;
		func = [temp_right](VarNode *var_left, SSNode *exp) {
			IRNode *ir = NULL;
			if (exp == NULL)
				ir = new IRNode(IRType::assign, IRAType::temp, temp_right, IRAType::var, var_left);
			return ir;
		};
	}
	else if (assign_right->type == SSType::int_const) {
		int int_const_right = assign_right->int_val;
		func = [int_const_right](VarNode *var_left, SSNode *exp) {
			IRNode *ir = NULL;
			if (exp == NULL)
				ir = new IRNode(IRType::assign, IRAType::int_imm, int_const_right, IRAType::var, var_left);
			return ir;
		};
	}
	else if (assign_right->type == SSType::float_const) {
		float float_const_right = assign_right->float_val;

		func = [float_const_right](VarNode *var_left, SSNode *exp) {
			IRNode *ir = NULL;
			//std::cout << float_const_right << " asdddddddddddddddddddddddddddddddddddddddd" << std::endl;
			if (exp == NULL)
				ir = new IRNode(IRType::assign, IRAType::float_imm, float_const_right, IRAType::var, var_left);
			return ir;
		};
	}

	for (int i = start_index; i < fin_index - 1; i++) {
		SSNode *left = ss_get(i);

		if (left->type == SSType::identifier) {
			VarNode *id = NULL;
			_handle_var_undecl(id, left)
			addIRNode(func(id, NULL));
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
		bool has_add = false, has_float_const = false, has_const = false, need_temp = false, use_const = false;
		SSNode *arg_add = NULL, *first_sub_arg = NULL, *first_ti_arg = NULL;
		int temp_sum = -1;
		int first_sub_arg_index = 0, first_ti_arg_index = 0;

		for (int i = start_index; i < fin_index; i++) {

			if (ss_get(i)->type == SSType::float_const) {
				if (i == start_index || ss_get(i - 1)->type == SSType::add)
					x_const += ss_get(i)->float_val;
				else
					x_const -= ss_get(i)->float_val;

				has_float_const = true;
				has_const = true;
			}
			else if (ss_get(i)->type == SSType::int_const) {
				if (i == start_index || ss_get(i - 1)->type == SSType::add)
					x_const += ss_get(i)->int_val;
				else
					x_const -= ss_get(i)->int_val;

				has_const = true;
			}
			else if (ss_get(i)->type == SSType::sub && first_sub_arg == NULL && !has_add && !SSNode::isConst(ss_get(i + 1)->type)) {
				first_sub_arg = ss_get(i + 1);
				first_sub_arg_index = i + 1;
			}
			else if (ss_get(i)->type == SSType::identifier || ss_get(i)->type == SSType::temp_var) {
				if (need_temp == false)
					temp_sum = temp_top_index++;
				need_temp = true;


				if (first_ti_arg == NULL) {
					first_ti_arg = ss_get(i);
					first_ti_arg_index = i;
				}

				if (!has_add && (i == start_index || ss_get(i - 1)->type == SSType::add)) {
					has_add = true;
					arg_add = ss_get(i);
				}
				//if(has_add && first_sub_arg != NULL)
			}
		}

		if (!need_temp) {
			for (int i = start_index; i < fin_index; i++)
				ss_pop();

			if (has_float_const) {
				ss_push(new SSNode(SSType::float_const, 0, x_const));
			}
			else {
				ss_push(new SSNode(SSType::int_const, 0, (int)x_const));
			}

			return true;

		}
		//
		int it_start;
		IRNode *ir = NULL;

		if (first_sub_arg != NULL) {
			//std::cout << "HADDEXP 1" << std::endl;
			if (first_sub_arg->type == SSType::identifier) {
				/*VarNode *var = stm->find(first_sub_arg->string_val);
				if (var == NULL) {
					return false;
				}*/
				VarNode *var = NULL;
				_handle_var_undecl(var, first_sub_arg)
				if (has_add) {
					if (arg_add->type == SSType::identifier) {
						/*VarNode *var2 = stm->find(first_sub_arg->string_val);
						if (var2 == NULL) {
							return false;
						}*/
						VarNode *var2 = NULL;
						_handle_var_undecl(var2, arg_add)
						ir = new IRNode(IRType::sub, IRAType::var, var, IRAType::var, var2, IRAType::temp, temp_sum);
					}
					else {
						ir = new IRNode(IRType::sub, IRAType::var, var, IRAType::temp, arg_add->int_val, IRAType::temp, temp_sum);
					}
				}
				else {
					ir = new IRNode(IRType::sub, IRAType::var, var, IRAType::float_imm, x_const, IRAType::temp, temp_sum);
					use_const = true;
				}

			}
			else {
				//std::cout << "HADDEXP 1" << std::endl;
				if (has_add) {
					if (arg_add->type == SSType::identifier) {
						/*VarNode *var2 = stm->find(first_sub_arg->string_val);
						if (var2 == NULL) {
							return false;
						}*/
						VarNode *var2 = NULL;
						_handle_var_undecl(var2, arg_add)
						ir = new IRNode(IRType::sub, IRAType::temp, first_sub_arg->int_val, IRAType::var, var2, IRAType::temp, temp_sum);
					}
					else {
						ir = new IRNode(IRType::sub, IRAType::temp, first_sub_arg->int_val, IRAType::temp, arg_add->int_val, IRAType::temp, temp_sum);
					}
				}
				else {
					ir = new IRNode(IRType::sub, IRAType::temp, first_sub_arg->int_val, IRAType::float_imm, x_const, IRAType::temp, temp_sum);
					use_const = true;
				}
			}

			it_start = first_sub_arg_index + 2;
		}
		else {

			if (first_ti_arg->type == SSType::identifier) {
				/*VarNode *var = stm->find(first_ti_arg->string_val);
				if (var == NULL) {
					return false;
				}*/
				VarNode *var = NULL;
				_handle_var_undecl(var, first_ti_arg)
				if (first_ti_arg_index != fin_index - 1) {
					SSNode *arg_0 = ss_get(first_ti_arg_index + 2);
					if (arg_0->type == SSType::identifier) {
						/*VarNode *var2 = stm->find(arg_0->string_val);
						if (var2 == NULL) {
							return false;
						}*/
						VarNode *var2 = NULL;
						_handle_var_undecl(var2, arg_0)
						IRNode *ir = new IRNode(ss_get(first_ti_arg_index + 1)->type == SSType::add ? IRType::add : IRType::sub, IRAType::var, var2, IRAType::var, var, IRAType::temp, temp_sum);
					}
					else if (arg_0->type == SSType::temp_var) {
						ir = new IRNode(ss_get(first_ti_arg_index + 1)->type == SSType::add ? IRType::add : IRType::sub, IRAType::temp, arg_0->int_val, IRAType::var, var, IRAType::temp, temp_sum);
					}
					else {
						ir = new IRNode(IRType::add, IRAType::float_imm, x_const, IRAType::var, var, IRAType::temp, temp_sum);
						use_const = true;
					}
				}
				else {
					ir = new IRNode(IRType::add, IRAType::float_imm, x_const, IRAType::var, var, IRAType::temp, temp_sum);
					use_const = true;
				}

			}
			else {

				if (first_ti_arg_index != fin_index - 1) {
					SSNode *arg_0 = ss_get(first_ti_arg_index + 2);
					if (arg_0->type == SSType::identifier) {
						/*VarNode *var2 = stm->find(arg_0->string_val);
						if (var2 == NULL) {
							return false;
						}*/
						VarNode *var2 = NULL;
						_handle_var_undecl(var2, arg_0)
						ir = new IRNode(ss_get(first_ti_arg_index + 1)->type == SSType::add ? IRType::add : IRType::sub, IRAType::var, var2, IRAType::temp, first_ti_arg->int_val, IRAType::temp, temp_sum);
					}
					else if (arg_0->type == SSType::temp_var) {
						ir = new IRNode(ss_get(first_ti_arg_index + 1)->type == SSType::add ? IRType::add : IRType::sub, IRAType::temp, arg_0->int_val, IRAType::temp, first_ti_arg->int_val, IRAType::temp, temp_sum);
					}
					else {
						ir = new IRNode(IRType::add, IRAType::float_imm, x_const, IRAType::temp, first_ti_arg->int_val, IRAType::temp, temp_sum);
						use_const = true;
					}
				}
				else {
					ir = new IRNode(IRType::add, IRAType::float_imm, x_const, IRAType::temp, first_ti_arg->int_val, IRAType::temp, temp_sum);
					use_const = true;
				}
			}
			it_start = ss_get(start_index)->type == SSType::sub ? start_index + 5 : start_index + 4;
		}

		addIRNode(ir);


		for (int i = it_start; i < fin_index; i++) {
			SSNode *arg = ss_get(i);

			if (first_sub_arg != NULL && arg == arg_add)
				continue;

			IRType oper_type = ss_get(i - 1)->type == SSType::add ? IRType::add : IRType::sub;

			if (arg->type == SSType::identifier) {
				/*VarNode *var = stm->find(arg->string_val);
				if (var == NULL) {
					return false;
				}*/
				VarNode *var = NULL;
				_handle_var_undecl(var, arg)
				ir = new IRNode(oper_type, IRAType::var, var, IRAType::temp, temp_sum, IRAType::temp, temp_sum);

			}
			else if (arg->type == SSType::temp_var) {
				ir = new IRNode(oper_type, IRAType::temp, arg->int_val, IRAType::temp, temp_sum, IRAType::temp, temp_sum);
			}

			addIRNode(ir);
		}

		if (!use_const && has_const) {

			ir = new IRNode(IRType::add, IRAType::float_imm, x_const, IRAType::temp, temp_sum, IRAType::temp, temp_sum);
			addIRNode(ir);
		}

		

		ss_push(new SSNode(SSType::temp_var, 0, temp_sum));
	}

	return true;
}

bool IRCreator::handle_multiplicative_exp() {


	int start_index = sp_top(), fin_index = ss_len();
	//int len = (ss_len() - sp_top() - 1) / 2;

	if (fin_index - start_index == 1) {

	}
	else {

		float x_const = 1;
		bool has_float_const = false, has_const = false;
		int num = 0;

		int t0, t1, t2;
		VarNode *v0, *v1, *v2;
		t0 = t1 = t2 = -1;
		v0 = v1 = v2 = NULL;
		SSNode *zz_s = NULL;
		SSType zz_t;
		IRNode *ir = NULL;
		IRNode *ir_with_const = NULL;

		for (int i = start_index; i < fin_index; i += 2) {

			SSNode *arg = ss_get(i);
			SSType oper_type;
			if (i == start_index) {
				oper_type = SSType::mult;
			}
			else {
				oper_type = ss_get(i - 1)->type;
			}

			if (SSNode::isConst(arg->type)) {
				has_const = true;
				has_float_const = (arg->type == SSType::float_const) || has_float_const;
				float c = arg->type == SSType::float_const ? arg->float_val : arg->int_val;
				if (oper_type == SSType::mult)
					x_const *= c;
				else
					x_const /= c;
			}
			else if (arg->type == SSType::identifier) {

				if (num < 1) {
					v1 = stm->find(arg->string_val);
					if (v1 == NULL) {
						return false;
					}
					t2 = temp_top_index++;
					//std::cout << arg->string_val << " " << t2 << std::endl;
					zz_s = arg;
					zz_t = oper_type;
				}
				else {

					IRType t = oper_type == SSType::mult ? IRType::mult : IRType::div;

					/*VarNode *var_a0 = stm->find(arg->string_val);
					if (var_a0 == NULL) {
						return false;
					}*/
					VarNode *var_a0 = NULL;
					_handle_var_undecl(var_a0, arg)

					if (num == 1) {
						SSNode *a1 = ss_get(i - 2);
						// 

						if (a1->type == SSType::identifier) {

							ir = new IRNode(t, IRAType::var, var_a0, IRAType::var, v1, IRAType::temp, t2);
						}
						else if (a1->type == SSType::temp_var) {
							ir = new IRNode(t, IRAType::var, var_a0, IRAType::temp, a1->int_val, IRAType::temp, t2);
						}
						else {
							ir = new IRNode(t, IRAType::var, var_a0, IRAType::float_imm, 0, IRAType::temp, t2);
							ir_with_const = ir;
						}

					}
					else {
						ir = new IRNode(t, IRAType::var, var_a0, IRAType::temp, t2, IRAType::temp, t2);
					}

					addIRNode(ir);

				}
				num++;
			}
			else if (arg->type == SSType::temp_var) {
				if (num < 1) {
					t2 = temp_top_index++;
					zz_s = arg;
					zz_t = oper_type;
				}
				else {
					IRType t = oper_type == SSType::mult ? IRType::mult : IRType::div;
					if (num == 1) {
						SSNode *a1 = ss_get(i - 2);
						if (a1->type == SSType::identifier) {
							ir = new IRNode(t, IRAType::temp, arg->int_val, IRAType::var, v1, IRAType::temp, t2);
						}
						else if (a1->type == SSType::temp_var) {
							ir = new IRNode(t, IRAType::temp, arg->int_val, IRAType::temp, a1->int_val, IRAType::temp, t2);
						}
						else {
							ir = new IRNode(t, IRAType::temp, arg->int_val, IRAType::float_imm, 0, IRAType::temp, t2);
							ir_with_const = ir;
						}
					}
					else {
						ir = new IRNode(t, IRAType::temp, arg->int_val, IRAType::temp, t2, IRAType::temp, t2);
					}

					addIRNode(ir);
				}
				num++;
			}

		}

		// pop


		if (num == 0) {// 1个以上的const_exp
			for (int i = start_index; i < fin_index; i++) {
				ss_pop();
			}
			if (has_float_const) {
				ss_push(new SSNode(SSType::float_const, 0, x_const));
			}
			else {
				ss_push(new SSNode(SSType::int_const, 0, (int)x_const));
			}
		}
		else if (num == 1) {
			//std::cout << "asdasd " << zz_s->string_val << " " << (int)zz_s->type << std::endl;
			IRType type = (zz_t == SSType::mult) ? IRType::mult : IRType::div;
			if (zz_s->type == SSType::identifier) {
				//std::cout << "asdasd" << std::endl;
				ir = new IRNode(type, IRAType::var, stm->find(zz_s->string_val), IRAType::float_imm, has_float_const ? x_const : (int)x_const, IRAType::temp, t2);
			}
			else {
				//std::cout << "t2 " << t2 << std::endl;
				ir = new IRNode(type, IRAType::temp, zz_s->int_val, IRAType::float_imm, has_float_const ? x_const : (int)x_const, IRAType::temp, t2);
			}
			for (int i = start_index; i < fin_index; i++) {
				ss_pop();
			}
			ss_push(new SSNode(SSType::temp_var, 0, t2));
			addIRNode(ir);
		}
		else {
			for (int i = start_index; i < fin_index; i++) {
				ss_pop();
			}
			if (has_const) {
				if (ir_with_const == NULL) {
					ir = new IRNode(IRType::mult, IRAType::float_imm, has_float_const ? x_const : (int)x_const, IRAType::temp, t2, IRAType::temp, t2);
					addIRNode(ir);
				}
				else {
					IRArg *a = new IRArg;
					if (has_float_const) {
						a->type = IRAType::float_imm;
						a->float_imm = x_const;
					}
					else {
						a->type = IRAType::int_imm;
						a->int_imm = (int)x_const;
					}
					ir_with_const->setArg(1, a);
				}
			}
			ss_push(new SSNode(SSType::temp_var, 0, t2));
		}
	}
	return true;
}

