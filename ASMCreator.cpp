#include "ASMCreator.h"
#include "FrontEnd\SymbolTable.h"
#include <string>

ASMCreator::ASMCreator(FrontEndInterface *f, RegAllocator *r, fstream &o) : FEI(f), alloc(r), out(o) {
	jump_strs[0] = "beq ";
	jump_strs[1] = "bne ";
	jump_strs[2] = "bge ";
	jump_strs[3] = "ble ";
	jump_strs[4] = "bgt ";
	jump_strs[5] = "blt ";
}

ASMCreator::~ASMCreator() {
}

void ASMCreator::create_head() {
	SymbolTable *whole_table = FEI->whole_table;

	//fprintf(save_fp, ".data\n");
	out << ".data" << endl;

	/**/

	int ir_num = FEI->ir_num;
	for (int i = 0; i < ir_num; i++) {
		IRNode *ir = FEI->getIR(i);
		if (ir->type == IRType::assign) {
			VarNode *n = ir->args[1].getVar();
			n->flag_global_scan = true;
			out << n->name << ": " << ".word " << ir->args[0].int_imm << endl;
		}
		else {
			break;
		}
	}

	whole_table->traverse([&](VarNode *n) {
		if(!n->flag_global_scan)
			out << n->name << ": " << ".word " << 0 << endl;
		n->at_reg = false;
		
		//std::cout << n->name << std::endl;
	});

	out << ".text" << endl;
	out << ".globl main" << endl;
}

void ASMCreator::create_block(int start, int end) {
	for (int i = start; i < end; i++) {
		IRNode *ir = FEI->getIR(i);
		LabelNode *label = NULL;
		label = FEI->getLabel(ir);
		
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
			out << "L" << label->index << ": " << endl;
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
			case IRType::ret:
				create_return();
				break;
			case IRType::func_call:
				create_call(ir);
				break;
			default:
				break;
			}
		}
	}
}

void ASMCreator::handle_t9(int imm) {
	if (alloc->t(9) != NULL)
		out << "sw " << "$t9, " << alloc->t(9)->stack_address << endl;
	out << "li " << "$t9, " << imm << endl;
}

int ASMCreator::load_store(IRNode *ir, int i) {
	if (ir->args[i].type == IRAType::var) {
		VarNode *var = ir->args[i].getVar();
		if (var->level == 0) {
			if (!var->at_reg) {
				VarNode *old = alloc->s(7);
				if (old != NULL) {
					if(old->level == 0)
						out << "sw " << "$s7, " << old->name << endl;
					else
						out << "sw " << "$s7, " << old->stack_address << "($sp)" << endl;
					old->at_reg = false;
				}

				out << "lw " << "$s7, " << var->name << endl;
				var->at_reg = true;
				var->reg_index = 7;
			}
			return 23;
		}
		else {
			if (!var->at_reg) {
				VarNode *old = alloc->s(7);
				if (old != NULL) {
					if (old->level == 0)
						out << "sw " << "$s7, " << old->name << endl;
					else
						out << "sw " << "$s7, " << old->stack_address << "($sp)" << endl;
					old->at_reg = false;
				}
					
				out << "lw " << "$s7, " << var->stack_address << "($sp)" << endl;
				var->at_reg = true;
				var->reg_index = 7;
				return 23;
			}
			else {
				return var->reg_index + 16;
			}
		}

	}
	else {
		TempNode *temp = ir->args[i].temp;
		if (!temp->at_reg) {
			TempNode *old = alloc->t(8);
			if (old != NULL) {
				out << "sw " << "$t8, " << old->stack_address << "($sp)" << endl;
				old->at_reg = false;
			}
				
			out << "lw " << "$t8, " << temp->stack_address << "($sp)" << endl;
			temp->at_reg = true;
			temp->reg_index = 8;
			return 24;
		}
		else {
			return temp->reg_index + 8;
		}
	}
}

std::string &ASMCreator::to_arg_str(IRNode *ir, int i) {
	int ret = load_store(ir, i);
	if (ret == -1) {
		return ir->args[i].getVar()->name;
	}
	else {
		std::string s = std::string("$");
		s += ret;
		return s;
	}
}

void ASMCreator::create_assign(IRNode *ir) {
	IRAType res_type = ir->args[0].type;

	int dest_reg = load_store(ir, 1);

	if (res_type == IRAType::int_imm) {
		int imm = ir->args[0].int_imm;
		if(imm == 0)
			out << "li " << "$" << dest_reg << ", $0" << endl;
		else
			out << "li " << "$" << dest_reg << ", " << imm << endl;
		//out << "li " << to_arg_str(ir, 1) << ", " << ir->args[0].int_imm << endl;
	}
	else {
		int res_reg = load_store(ir, 0);
		out << "move " << "$" << dest_reg << ", $" << res_reg << endl;
		//out << "li " << to_arg_str(ir, 1) << ", " << to_arg_str(ir, 0) << endl;
	}
	
}

void ASMCreator::create_asmd(IRNode *ir) {// 中间代码保证不会有 a := imm op imm 的形式, 只有imm op a / a op b
	int dest_reg = load_store(ir, 2), a0_reg = load_store(ir, 0);
	if (ir->type == IRType::add || ir->type == IRType::sub) {
		if (ir->args[1].type == IRAType::int_imm) {
			int imm = ir->args[1].int_imm;
			
			if (ir->type == IRType::add) {
				if(imm = 0)
					out << "addi " << "$" << dest_reg << ", $" << a0_reg << ", $0" << endl;
				else
					out << "addi " << "$" << dest_reg << ", $" << a0_reg << ", " << imm << endl;
			}
			else {
				if (imm = 0)
					out << "subi " << "$" << dest_reg << ", $" << a0_reg << ", $0" << endl;
				else
					out << "subi " << "$" << dest_reg << ", $" << a0_reg << ", " << imm << endl;
				//out << "subi " << "$" << dest_reg << ", $" << a0_reg << ", " << imm << endl;
			}
		}
		else {
			int a1_reg = load_store(ir, 1);
			if (ir->type == IRType::add) {
				out << "add " << "$" << dest_reg << ", $" << a0_reg << ", $" << a1_reg << endl;
			}
			else {
				out << "sub " << "$" << dest_reg << ", $" << a0_reg << ", $" << a1_reg << endl;
			}
		}
	}
	else {
		if (ir->args[1].type == IRAType::int_imm) {
			int imm = ir->args[1].int_imm;

			if (imm == 0) {
				if (ir->type == IRType::mult) {
					out << "mul " << "$" << dest_reg << ", $" << a0_reg << ", $0" << endl;
				}
				else {
					out << "div " << "$" << dest_reg << ", $" << a0_reg << ", $0" << endl;
				}
			}
			else {
				handle_t9(imm);

				if (ir->type == IRType::mult) {
					out << "mult " << "$" << a0_reg << ", $t9" << endl;
				}
				else {
					out << "div " << "$" << a0_reg << ", $t9" << endl;
				}
			}

			out << "mflo " << "$" << dest_reg << endl;
		}
		else {
			int a1_reg = load_store(ir, 1);
			if (ir->type == IRType::mult) {
				out << "mul " << "$" << dest_reg << ", $" << a0_reg << ", $" << a1_reg << endl;
			}
			else {
				out << "div " << "$" << dest_reg << ", $" << a0_reg << ", $" << a1_reg << endl;
			}
		}
	}

}

void ASMCreator::create_print(IRNode *ir) {
	IRAType type = ir->args[0].type;
	if (type == IRAType::var) {
		VarNode *var = ir->args[0].getVar();
		if (var->at_reg) {
			out << "li $v0, 1" << endl;
			out << "move $a0, $s" << var->reg_index << endl;
		}
		else {

		}
	}
	else if (type == IRAType::temp) {
		TempNode *t = ir->args[0].temp;
		if (t->has_reg) {
			out << "li $v0, 1" << endl;
			out << "move $a0, $t" << t->reg_index << endl;
		}
	}
	else if (type == IRAType::int_imm) {
		out << "li $v0, 1" << endl;
		int imm = ir->args[0].int_imm;
		if(imm == 0)
			out << "li $a0, $0" << endl;
		else
			out << "li $a0, " << imm << endl;
	}
	else if (type == IRAType::char_imm) {
		out << "li $v0, 11" << endl;
		char c = ir->args[0].char_imm;
		
		out << "li $a0, " << (int)c << endl;
	}
	out << "syscall" << endl;
}

void ASMCreator::create_func(IRNode *ir) {
	FuncNode *func = ir->args[0].getFuc();
	//std::cout << "SSSSSSSSSSS";
	creating_def_func = func;
	out << func->name << ":" << endl;
	out << "addiu $sp, $sp, -" << func->size << endl;
	out << "sw $ra, " << func->size - 4 << "($sp)" << endl;
	out << "sw $fp, " << func->size - 8 << "($sp)" << endl;
	out << "move $fp, $sp" << endl;

}

void ASMCreator::create_jump(IRNode *ir) {// 优化 if 3 > 2的情况!!!!!!!!!
	if (ir->type == IRType::jump) {
		out << "j L" << ir->args[0].label->index << endl;
		out << "nop" << endl;
	}
	else {
		int label_index = ir->args[2].label->index;
		int str_index = (int)ir->type - (int)IRType::equal_jump;
		/*switch (ir->type) {
		case IRType::equal_jump: str; break;
		case IRType::unequal_jump: out << "bne "; break;
		case IRType::ge_jump: out << "bge "; break;
		case IRType::le_jump: out << "ble "; break;
		case IRType::less_jump: out << "blt "; break;
		case IRType::greater_jump: out << "bgt "; break;
		default:
			break;
		}*/
		int reg_2;
		if (ir->args[1].type == IRAType::int_imm) {// 优化 若此立即数低于16位， 直接b指令
			int imm = ir->args[1].int_imm;

			handle_t9(imm);
			//out << "$t9";
		}
		else {
			reg_2 = load_store(ir, 1);
			//out << "$" << reg;
		}
		if (ir->args[0].type == IRAType::int_imm) {// 优化 若此立即数低于16位， 直接b指令
			int imm = ir->args[0].int_imm;

			handle_t9(imm);
			out << jump_strs[str_index] << "$t9, ";
		}
		else {
			int reg = load_store(ir, 0);
			out << jump_strs[str_index] << "$" << reg << ", ";
		}

		
		if (ir->args[1].type == IRAType::int_imm) {// 优化 若此立即数低于16位， 直接b指令
			out << "$t9";
		}
		else {
			out << "$" << reg_2;
		}

		out << ", L" << ir->args[2].label->index << endl;
		out << "nop" << endl;
	}
	
}

void ASMCreator::create_call(IRNode *ir) {
	//
	creating_call_ir = ir;
	//
}

void ASMCreator::create_call_fin() {
	FuncNode *func = creating_call_ir->args[0].getFuc();
	creating_call_ir = NULL;
	now_a_reg_index = 0;
	out << "jal " << func->name << endl;
	out << "nop" << endl;
}

void ASMCreator::create_param_in(IRNode *ir) {
	if (now_a_reg_index != 4) {
		if (ir->args[0].type == IRAType::int_imm) {
			int imm = ir->args[0].int_imm;
			if(imm == 0)
				out << "li $a" << now_a_reg_index++ << ", $0" << endl;
			else
				out << "li $a" << now_a_reg_index++ << ", " << imm << endl;
		}
		else {
			int reg = load_store(ir, 0);
			out << "move $a" << now_a_reg_index++ << ", $" << reg << endl;
		}
	}
	else {

	}
	
}

void ASMCreator::create_return() {
	FuncNode *func = creating_def_func;

	out << "move $sp, $fp" << endl;
	out << "lw $ra, " << func->size - 4 << "($sp)" << endl;
	out << "lw $fp, " << func->size - 8 << "($sp)" << endl;
	out << "jr $ra" << endl;
	//out << "nop" << endl;
	out << "addiu $sp, $sp, " << func->size << endl;
}