#include "ASMCreator.h"
#include <string>


ASMOut::ASMOut(fstream &o, bool b) : fout(o), is_print_json(b) {
	state = ASMOS::start;
}

ASMOut::~ASMOut() {
}
ASMOut& operator<< (ASMOut &out, char *s) {
	if (out.state == ASMOS::start) {
		out.state = ASMOS::first;
		if (strcmp(s, "lw") == 0 || strcmp(s, "sw") == 0)
			out.ls = true;
		else
			out.ls = false;
		out.fout << s << " ";

		if (out.is_print_json) {
			if (out.is_frist_asm) {
				std::cout << "{";
				out.is_frist_asm = false;
			}
			else
				std::cout << ",{";
			std::cout << "\"op\":\"" << s << "\"";
		}
	}
	else if (out.state == ASMOS::first) {
		out.state = ASMOS::sec;
		out.fout << s;
		if (out.is_print_json) {
			std::cout << ",\"arg\":[\"" << s << "\"";
		}
	}
	else if (out.state == ASMOS::imm && out.ls) {
		out.fout << s;
		if (out.is_print_json) {
			std::cout << s << "\"";
		}
	}
	else {
		out.fout << ", " << s;
		if (out.is_print_json) {
			std::cout << ",\"" << s << "\"";
		}
	}

	return out;
}
ASMOut& operator<< (ASMOut &out, int imm) {
	out.state = ASMOS::imm;
	out.fout << ", " << imm;
	if (out.is_print_json) {
		if (out.ls) {
			std::cout << ",\"" << imm;
		}
		else {
			std::cout << "," << imm;
		}
	}
	return out;
}

ASMOut& operator<< (ASMOut &out, std::string &s) {
	if (out.state == ASMOS::start) {
		out.state = ASMOS::first;
		out.fout << s << " ";
		if (out.is_print_json) {
			if (out.is_frist_asm) {
				std::cout << "{";
				out.is_frist_asm = false;
			}
			else
				std::cout << ",{";
			std::cout << "\"op\":\"" << s << "\"";
		}
	}
	else if (out.state == ASMOS::first) {
		out.state = ASMOS::sec;
		out.fout << s;
		if (out.is_print_json) {
			std::cout << ",\"arg\":[\"" << s << "\"";
		}
	}
	else if (out.state == ASMOS::imm && out.ls) {
		out.fout << s;
		if (out.is_print_json) {
			std::cout << s << "\"";
		}
	}
	else {
		out.fout << ", " << s;
		if (out.is_print_json) {
			std::cout << ",\"" << s << "\"";
		}
	}

	return out;
}

void operator<< (ASMOut &out, basic_ostream<char, char_traits<char>>& (*_Pfn)(basic_ostream<char, char_traits<char>>&)) {
	
	out.ls = false;
	out.fout << endl;
	if (out.is_print_json) {
		if(out.state != ASMOS::first)
			std::cout << "]";
		std::cout << "}" << std::endl;
	}
	out.state = ASMOS::start;

}

ASMCreator::ASMCreator(FrontEndInterface *f, RegAllocator *r, fstream &o, bool print_json) : FEI(f), alloc(r), out(o, print_json) {
	help_buf = new char[100];

	if (print_json) {
		std::cout << ",\"ASM\":[" << std::endl;
	}
}

ASMCreator::~ASMCreator() {
}

void ASMCreator::create_head() {
	SymbolTable *whole_table = FEI->whole_table;

	//fprintf(save_fp, ".data\n");
	out << ".data" << endl;

	/**/

	int ir_num = FEI->ir_list.size();
	int fi = 0;
	for (int i = 0; i < ir_num; i++) {
		IRNode *ir = FEI->getIR(i);
		if (ir->type == IRType::assign) {
			VarNode *n = ir->args[1].getVar();
			n->flag_global_scan = true;
			sprintf(help_buf, "%s:.word %d", n->name.c_str(), ir->args[0].int_imm);

			out << help_buf << endl;
		}
		else {
			fi = i;
			break;
		}
	}
	

	whole_table->traverse([&](VarNode *n) {
		if (!n->flag_global_scan) {
			out << n->name + ": " + ".word " << 0 << endl;
		}
			
		n->at_reg = false;
		
		//std::cout << n->name << std::endl;
	});

	out << ".text" << endl;
	out << ".globl main" << endl;
	alloc->alloc(fi, ir_num);
	create_block(fi, ir_num);
	out.fin();
}

void ASMCreator::create_block(int start, int end) {
	for (int i = start; i < end; i++) {
		cur_ir_index = i;
		IRNode *ir = FEI->getIR(i);
		LabelNode *label = ir->label;
		//label = FEI->getLabel(ir);
		
		if (creating_call_ir != NULL) {
			if (ir->type != IRType::func_param_in) {
				create_call_fin();
			}
			else {
				create_param_in(ir);
				continue;
			}
		}

		if (label != NULL) {
			sprintf(help_buf, "L%d:", label->index);
			//out << std::string("L") + label->index + ":" << endl;
			out << help_buf << endl;
		}

		if (i == FEI->ir_num - 1) {
			out.ir_end = true;
		}

		if (ir->type >= IRType::add && ir->type <= IRType::div) {
			create_asmd(ir);
		}
		else if (ir->type >= IRType::equal_jump && ir->type <= IRType::jump) {
			create_jump(ir);
		}
		else {
			switch (ir->type) {
			case IRType::func:
				create_func(ir);
				break;
			case IRType::assign:
				create_assign(ir);
				break;
			case IRType::print:
				create_print(ir);
				break;
			case IRType::input:
				if (ir->args[0].type == IRAType::temp) {
					//create_input(ir, FEI->getIR(++i));
					create_input(ir, NULL);
				}
				else {
					create_input(ir, NULL);
				}
				
				break;
			case IRType::ret:
				create_return(ir);
				break;
			case IRType::func_call:
				create_call(ir);
				break;
			case IRType::array_assign:
				create_array_assign(ir);
				break;
			case IRType::array_use:
				create_array_use(ir);
				break;
			default:
				break;
			}
		}
	}
}

void ASMCreator::handle_t9(int imm) {
	if (alloc->t(9) != NULL)
		out << "sw" << "$t9" << alloc->t(9)->stack_address << endl;
	out << "li" << "$t9" << imm << endl;
}

void ASMCreator::handle_t9(VarNode *var, int add = -1) {
	if (alloc->t(9) != NULL)
		out << "sw" << "$t9" << alloc->t(9)->stack_address << endl;
	if (var->varType->index < 4) {
		out << "lw" << "$t9" << var_offset(var) << "($sp)" << endl;
	}
	else {
		if(add == -1)
			out << "addi" << "$t9" << "$sp" << var_offset(var) << endl;
		else
			out << "addi" << "$t9" << "$sp" << var_offset(var) + add << endl;
	}
	
}

int ASMCreator::var_offset(VarNode *var) {
	if (var->is_arg) {
		//std::cout << var->reg_index << " AAAAAAAA";
		return -(creating_def_func->size + var->arg_index * 4);
	}
	else {
		return -(creating_def_func->size - 8 - var->stack_address);
	}
}

int ASMCreator::var_offset(TempNode *temp) {
	return -(creating_def_func->size - 8 - temp->stack_address);
}

int ASMCreator::load_store(IRNode *ir, int i, bool use_t9 = false) {
	if (ir->args[i].type == IRAType::var) {
		VarNode *var = ir->args[i].getVar();
		if (var->level == 0) {
			
			if (!var->at_reg) {
				
				VarNode *old = alloc->s(7);
				if (old != NULL) {
					if(old->level == 0)
						out << "sw" << "$s7" << old->name << endl;
					else
						out << "sw" << "$s7" << var_offset(old) << "($sp)" << endl;
					old->at_reg = false;
				}
				if ((ir->type == IRType::assign && i == 1) || (i == 2 && ir->type >= IRType::add && ir->type <= IRType::div)) {
					//std::cout << "SSSSSSSSSSS" << cur_ir_index << std::endl;
					alloc->s(var); 
				}
				else if (use_t9) {
					handle_t9(var);
					return 25;
				}
				else {
					
					out << "lw" << "$s7" << var->name << endl;
					alloc->s(var);
				}
					
				var->at_reg = true;
				var->reg_index = 23;
			}
			return 23;
		}
		else {
			if (!var->at_reg) {
				VarNode *old = alloc->s(7);
				if (old != NULL) {
					if (old->level == 0)
						out << "sw" << "$s7" << old->name << endl;
					else
						out << "sw " << "$s7" << var_offset(old) << "($sp)" << endl;
					old->at_reg = false;
				}
				if (ir->type == IRType::assign && i == 1 || i == 2 && ir->type >= IRType::add && ir->type <= IRType::div) {
					alloc->s(var);
				}
				else if (use_t9) {
					handle_t9(var);
					return 25;
				}
				else {
					out << "lw " << "$s7" << var_offset(var) << "($sp)" << endl;
					alloc->s(var);
				}
					
				var->at_reg = true;
				var->reg_index = 23;
				return 23;
			}
			else {
				return var->reg_index;
			}
		}

	}
	else {
		TempNode *temp = ir->args[i].temp;
		if (!temp->at_reg) {
			TempNode *old = alloc->t(8);
			if (old != NULL) {
				out << "sw" << "$t8" << var_offset(old) << "($sp)" << endl;
				old->at_reg = false;
			}
				
			out << "lw" << "$t8" << var_offset(temp) << "($sp)" << endl;
			temp->at_reg = true;
			temp->reg_index = 24;
			return 24;
		}
		else {
			return temp->reg_index;
		}
	}
}

void ASMCreator::store(VarNode *var) {
	/*if(var->at_reg)
		out << "sw " << reg_strs[var->reg_index] << ", " << var_offset(var) << "($sp)" << endl;*/
	if (var->level == 0) {
		out << "sw" << "$s7" << var->name << endl;
	}
	else {
		out << "sw" << "$s7" << var_offset(var) << "($sp)" << endl;
	}
	
}

void ASMCreator::create_assign(IRNode *ir) {
	IRAType res_type = ir->args[0].type;

	int dest_reg = load_store(ir, 1);

	if (res_type == IRAType::int_imm) {
		int imm = ir->args[0].int_imm;
		if(imm == 0)
			out << "move" << reg_strs[dest_reg] << "$0" << endl;
		else
			out << "li" << reg_strs[dest_reg] << imm << endl;
	}
	else {
		int res_reg = load_store(ir, 0);
		out << "move" << reg_strs[dest_reg] << reg_strs[res_reg] << endl;
	}
	
}

void ASMCreator::create_asmd(IRNode *ir) {// 中间代码保证不会有 a := imm op imm 的形式
	int dest_reg = load_store(ir, 2); 
	if (ir->type == IRType::add || ir->type == IRType::sub) {// +- 若有imm, 则一定是 args[1]
		if (ir->args[1].type == IRAType::int_imm) {
			int imm = ir->args[1].int_imm;
			int a0_reg = load_store(ir, 0);
			if (ir->type == IRType::add) {
				if (imm == 0)
					out << "add" << reg_strs[dest_reg] << reg_strs[a0_reg] << "$0" << endl;
				else
					out << "addi" << reg_strs[dest_reg] << reg_strs[a0_reg] << imm << endl;
			}
			else {
				if (imm == 0)
					out << "sub" << reg_strs[dest_reg] << "$0" << reg_strs[a0_reg] << endl;
				else {
					handle_t9(imm);
					//out << "li" << "$v1" << imm << endl;
					out << "sub" << reg_strs[dest_reg] << "$t9" << reg_strs[a0_reg] << endl;
				}
					
			}
		}
		else {
			int a0_reg = load_store(ir, 1);
			int a1_reg = load_store(ir, 0, a0_reg == 23);
			if (ir->type == IRType::add) {
				out << "add" << reg_strs[dest_reg] << reg_strs[a0_reg] << reg_strs[a1_reg] << endl;
			}
			else {
				out << "sub" << reg_strs[dest_reg] << reg_strs[a0_reg] << reg_strs[a1_reg] << endl;
			}
		}
	}
	else {// 应该没有0
		if (ir->args[0].type == IRAType::int_imm) {
			int imm = ir->args[0].int_imm;
			int a0_reg = load_store(ir, 1);
			if (imm == 0) {
				if (ir->type == IRType::mult) {
					out << "move" << reg_strs[dest_reg] << "$0" << endl;
					//out << "mul" << reg_strs[dest_reg] << reg_strs[a0_reg] << "$0" << endl;
				}
				else {
					out << "div" << reg_strs[dest_reg] << reg_strs[a0_reg] << "$0" << endl;
				}
			}
			else {
				handle_t9(imm);

				if (ir->type == IRType::mult) {
					out << "mult" << reg_strs[a0_reg] << "$t9" << endl;
				}
				else {
					out << "div" << reg_strs[a0_reg] << "$t9" << endl;
				}
			}

			out << "mflo" << reg_strs[dest_reg] << endl;
		}
		else if (ir->args[1].type == IRAType::int_imm) {
			int imm = ir->args[1].int_imm;
			int a1_reg = load_store(ir, 0);
			if (imm == 0) {
				out << "move" << reg_strs[dest_reg] << "$0" << endl;
			}
			else {
				handle_t9(imm);

				if (ir->type == IRType::mult) {
					out << "mult" << "$t9" << reg_strs[a1_reg] << endl;
				}
				else {
					out << "div" << "$t9" << reg_strs[a1_reg] << endl;
				}
			}

			out << "mflo" << reg_strs[dest_reg] << endl;
		}
		else {
			int a0_reg = load_store(ir, 1);
			int a1_reg = load_store(ir, 0, a0_reg == 23);
			if (ir->type == IRType::mult) {
				out << "mul" << reg_strs[dest_reg] << reg_strs[a0_reg] << reg_strs[a1_reg] << endl;
			}
			else {
				out << "div" << reg_strs[dest_reg] << reg_strs[a0_reg] << reg_strs[a1_reg] << endl;
			}
		}
	}

}

void ASMCreator::create_print(IRNode *ir) {
	IRAType type = ir->args[0].type;

	bool is_a0 = type == IRAType::var && ir->args[0].getVar()->arg_index == 0;
	if (is_a0) {// 占用a0寄存器
		out << "li" << "$v0" << 1 << endl;
		out << "syscall" << endl;
	}
	else {
		// save a
		if (creating_def_func->param_num > 0)
			out << "sw" << "$a0" << creating_def_func->size << "($sp)" << endl;
		if (type == IRAType::var) {
			VarNode *var = ir->args[0].getVar();
			if (var->at_reg) {
				out << "li" << "$v0" << "1" << endl;
				out << "move" << "$a0" << reg_strs[var->reg_index] << endl;
			}
			else {
				int r = load_store(ir, 0);
				out << "li" << "$v0" << 1 << endl;
				out << "move" << "$a0" << reg_strs[r] << endl;
			}
		}
		else if (type == IRAType::temp) {
			TempNode *t = ir->args[0].temp;
			if (t->at_reg) {
				out << "li" << "$v0" << 1 << endl;
				out << "move" << "$a0" << reg_strs[t->reg_index] << endl;
			}
		}
		else if (type == IRAType::int_imm) {
			out << "li" << "$v0" << "1" << endl;
			int imm = ir->args[0].int_imm;
			if (imm == 0)
				out << "li" << "$a0" << "$0" << endl;
			else
				out << "li" << "$a0" << imm << endl;
		}
		else if (type == IRAType::char_imm) {
			out << "li" << "$v0" << 11 << endl;
			char c = ir->args[0].char_imm;

			out << "li" << "$a0" << (int)c << endl;
		}
		out << "syscall" << endl;
		if(creating_def_func->param_num > 0)
			out << "lw" << "$a0" << creating_def_func->size << "($sp)" << endl;
	}

	
}

void ASMCreator::create_input(IRNode *ir, IRNode *next) {

	if (ir->args[0].type == IRAType::var) {
		VarNode *var = ir->args[0].getVar();
		out << "li" << "$v0" << 5 << endl;
		out << "syscall" << endl;
		if (var->at_reg) {
			out << "move" << reg_strs[var->reg_index] << "$v0" << endl;
		}
		else {
			if (var->level == 0) {
				out << "sw" << "$v0" << var_offset(var) << "($sp)" << endl;
			}
			else {
				out << "sw" << "$v0" << var->name << endl;
			}
			
			//out << "move" << reg_strs[r] << "$v0" << endl;
		}
	}
	else {
		TempNode *t = ir->args[0].temp;
		out << "li" << "$v0" << 5 << endl;
		out << "syscall" << endl;
		if (t->at_reg) {
			out << "move" << reg_strs[t->reg_index] << "$v0" << endl;
		}
		else {
			out << "sw" << "$v0" << var_offset(t) << endl;
			//out << "move" << reg_strs[r] << "$v0" << endl;
		}
	}
	
}

void ASMCreator::create_func(IRNode *ir) {
	FuncNode *func = ir->args[0].getFuc();
	//std::cout << "SSSSSSSSSSS";
	creating_def_func = func;
	func->size += 4;
	out << func->name + ":" << endl;
	out << "addiu" << "$sp" << "$sp" << func->size << endl;
	out << "sw" << "$ra" << -func->size + 4 << "($sp)" << endl;
	out << "sw" << "$fp" << -func->size + 8 << "($sp)" << endl;
	out << "move" << "$fp" << "$sp" << endl;

}

void ASMCreator::create_jump(IRNode *ir) {// 优化 if 3 > 2的情况!!!!!!!!!
	if (ir->type == IRType::jump) {
		sprintf(help_buf, "L%d", ir->args[0].label->index);
		out << "j" << help_buf << endl;
		out << "nop" << endl;
	}
	else {
		int jump_index = (int)ir->type - (int)IRType::equal_jump;
		int reg_1_index, reg_2_index;
		if (ir->args[1].type == IRAType::int_imm) {// 优化 若此立即数低于16位， 直接b指令
			int imm = ir->args[1].int_imm;
			if (imm == 0)
				reg_2_index = 0;
			else {
				handle_t9(imm);
				reg_2_index = 25;
			}
		}
		else {
			reg_2_index = load_store(ir, 1);
		}
		if (ir->args[0].type == IRAType::int_imm) {// 优化 若此立即数低于16位， 直接b指令
			int imm = ir->args[0].int_imm;
			if (imm == 0)
				reg_1_index = 0;
			else {
				handle_t9(imm);
				reg_1_index = 25;
			}
		}
		else {
			reg_1_index = load_store(ir, 0);
		}
		sprintf(help_buf, "L%d", ir->args[2].label->index);
		out << jump_strs[jump_index] << reg_strs[reg_1_index] << reg_strs[reg_2_index] << help_buf << endl;
		out << "nop" << endl;
	}
	
}

void ASMCreator::create_call(IRNode *ir) {
	//
	creating_call_ir = ir;
	FuncNode *func = ir->args[0].getFuc();
	//
	SymbolTable *table = ir->scope;
	table->back_traverse([&](VarNode *n) {
		if (n->at_reg && !n->is_useless) {
			out << "sw" << reg_strs[n->reg_index] << var_offset(n) << "($sp)" << endl;
		}
		//std::cout << "SWWWWWWW 1 " << n->name << std::endl;
	});

	for (auto t : func->need_store_temp_list) {
		out << "sw" << reg_strs[t->reg_index] << var_offset(t) << "($sp)" << endl;
	}

	// save s7
	VarNode *s7 = alloc->s(7);
	if (s7 != NULL)
		store(s7);

}

void ASMCreator::create_call_fin() {
	FuncNode *func = creating_call_ir->args[0].getFuc();
	
	
	//
	out << "jal" << func->name << endl;
	out << "nop" << endl;
	
	/*for (int i = 0; i < creating_def_func->param_num; i++) {
		VarNode *a = creating_def_func->paraList[i];
		if (a->at_reg && !a->is_useless) {
			out << "lw" << reg_strs[a->reg_index] << var_offset(a) << "($fp)" << endl;
		}
	}*/
	SymbolTable *table = creating_call_ir->scope;
	table->back_traverse([&](VarNode *n) {
		if (n->has_reg && !n->is_useless) {
			out << "lw" << reg_strs[n->reg_index] << var_offset(n) << "($sp)" << endl;
		}
		//std::cout << "LWWWWWWW " << n->name << std::endl;
	});
	for (auto t : func->need_store_temp_list) {
		out << "lw" << reg_strs[t->reg_index] << var_offset(t) << "($sp)" << endl;
	}
	if (func->retType->index != 2) {
		int r = load_store(creating_call_ir, 1);
		out << "move" << reg_strs[r] << "$v0" << endl;
	}
	creating_call_ir = NULL;
	now_a_reg_index = 0;
}

void ASMCreator::create_param_in(IRNode *ir) {// !!!!!!!!!!!!!
	if (now_a_reg_index != 4) {
		if (ir->args[0].type == IRAType::int_imm) {
			int imm = ir->args[0].int_imm;
			if(imm == 0)
				out << "li" << reg_strs[4 + now_a_reg_index++] << "$0" << endl;
			else
				out << "li" << reg_strs[4 + now_a_reg_index++] << imm << endl;
		}
		else {
			int reg = load_store(ir, 0);
			out << "move" << reg_strs[4 + now_a_reg_index++] << reg_strs[reg] << endl;
		}
	}
	else {

	}
	
}

void ASMCreator::create_return(IRNode *ir) {
	FuncNode *func = creating_def_func;
	
	if (func->retType->index == 2) {// void

	}
	else if (ir->args[0].type == IRAType::int_imm) {
		int imm = ir->args[0].int_imm;
		if (imm == 0)
			out << "move" << "$v0" << "$0" << endl;
		else
			out << "li" << "$v0" << imm << endl;
	}
	else {
		int res_reg = load_store(ir, 0);
		out << "move" << "$v0" << reg_strs[res_reg] << endl;
	}

	//out << "move" <<  "$sp" << "$sp" << endl;
	out << "lw" << "$ra" << -func->size + 4 << "($sp)" << endl;
	out << "lw" << "$fp" << -func->size + 8 << "($sp)" << endl;
	out << "addiu" << "$sp" << "$sp" << -func->size << endl;
	out << "jr" << "$ra" << endl;
	out << "nop" << endl;
	
}

void ASMCreator::create_array_assign(IRNode *ir) {
	VarNode *arr = ir->args[2].getVar();
	
	if (ir->args[1].type != IRAType::int_imm) {
		handle_t9(arr);// 数组基地址在 $t9
		int r = load_store(ir, 1);
		out << "sll" << "$v1" << reg_strs[r] << 2 << endl;// *4
		out << "add" << "$v1" << "$t9" << "$v1" << endl;// 元素地址在 $v1

		if (ir->args[0].type != IRAType::int_imm) {
			int r = load_store(ir, 0);

			out << "sw" << reg_strs[r] << "($v1)" << endl;
		}
		else {
			handle_t9(ir->args[0].int_imm);
			out << "sw" << "$t9" << "($v1)" << endl;
		}
	}
	else {
		//handle_t9(arr, ir->args[1].int_imm*4);// 元素地址在 $t9
		if (ir->args[0].type != IRAType::int_imm) {
			int r = load_store(ir, 0);

			out << "sw" << reg_strs[r] << var_offset(arr) + ir->args[1].int_imm * 4 << "($sp)" << endl;
		}
		else {
			handle_t9(ir->args[0].int_imm);
			out << "sw" << "$t9" << var_offset(arr) + ir->args[1].int_imm * 4 << "($sp)" << endl;
		}
	}

	
}

void ASMCreator::create_array_use(IRNode *ir) {
	VarNode *arr = ir->args[1].getVar();

	if (ir->args[0].type != IRAType::int_imm) {
		handle_t9(arr);// 数组基地址在 $t9
		int r = load_store(ir, 0);
		out << "sll" << "$v1" << reg_strs[r] << 2 << endl;// *4
		out << "add" << "$t9" << "$t9" << "$v1" << endl;// 元素地址在 $t9

		r = load_store(ir, 2);
		out << "lw" << reg_strs[r] << "($t9)" << endl;
	}
	else {
		int r = load_store(ir, 2);
		out << "lw" << reg_strs[r] << var_offset(arr) + ir->args[0].int_imm * 4 << "($sp)" << endl;
	}
}