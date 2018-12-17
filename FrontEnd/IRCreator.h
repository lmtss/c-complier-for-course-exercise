#pragma once
#include <cstdarg>
#include "SS.h"
#include "SymbolTable.h"
#include "Error.h"
#include "token.h"
#include <iostream>
#include <vector>
#include <type_traits>
#include <map>

#define _handle_var_undecl(vptr, sptr) if((vptr = stm->find(sptr->string_val)) == NULL){ em->addEN(stm->getCurFunc(), sptr->code_line, sptr->string_val, ErrorType::undeclared); return false;}
#define _ir_set_arg(b) if(b == false) return false;
#define _set_arg(ir,index,node) if(set_arg(ir, index, node) == false) return false;

extern STManager* stManager;

enum class IRAType {
	int_imm, float_imm, var, temp, NONE, jump_label, func, char_imm
};
enum class IRType {
	add, sub, mult, div, assign, ret, func, func_call, func_param_in,
	equal_jump, unequal_jump, ge_jump, le_jump, greater_jump, less_jump, jump,
	print, null, input
};

struct IRNode;

struct LabelNode {
	IRNode *from, *target;
	int index = -1;
	LabelNode(IRNode *t, IRNode *f = NULL) {
		target = t;
		from = f;
	}
	void set() {
		
	}

	//static int label_index;
};


struct IRArg {
	int int_imm;
	float float_imm;
	int temp_index;
	char char_imm;
	TempNode *temp;
	IDNode *id;
	LabelNode *label;
	IRAType type = IRAType::NONE;
	void print() {
		switch (type) {
		case IRAType::int_imm:
			std::cout << int_imm;
			break;
		case IRAType::float_imm:
			std::cout << float_imm;
			break;
		case IRAType::char_imm:
			if (char_imm != '\n')
				std::cout << '\'' << char_imm << '\'';
			else
				std::cout << "\'\\n\'";
			break;
		case IRAType::var:
			std::cout << id->name;
			break;
		case IRAType::temp:
			std::cout << 't' << temp->index;
			break;
		case IRAType::jump_label:
			std::cout << 'L' << label->index;
			break;
		case IRAType::func:
			std::cout << ((FuncNode *)id)->name << " ret " << ((FuncNode *)id)->retType->name;
			break;
		default:
			break;
		}
		std::cout << " ";
	}
	VarNode *getVar() { return (VarNode *)id; }
	FuncNode *getFuc() { return (FuncNode *)id; }
	bool operator == (const IRArg & b) const {
		if (type == b.type) {
			switch (type) {
			case IRAType::int_imm:
				return int_imm == b.int_imm;
				break;
			case IRAType::var:
				return id == b.id;
				break;
			case IRAType::temp:
				return temp == b.temp;
				break;
			default:
				return false;
				break;
			}
		}
		return false;
	}

	bool operator < (const IRArg & b) const {
		if (type == b.type) {
			switch (type) {
			case IRAType::int_imm:
				return int_imm < b.int_imm;
				break;
			case IRAType::var:
				return id < b.id;
				break;
			case IRAType::temp:
				return temp < b.temp;
				break;
			default:
				return false;
				break;
			}
		}
		else {
			return type < b.type;
		}
	}
};

struct TempType {
	int temp_index = 0;
	TypeNode *type = NULL;
	TempType(int ti) {
		temp_index = ti;
	}
};

struct IRNode {
	IRArg args[3];
	IRType type;
	IRNode *next, *front;
	LabelNode *label = NULL;
	
	int index_for_lsa = 0;
	SymbolTable *scope = NULL;
	IRNode() {
		type = IRType::add;
		next = NULL;
		front = NULL;
	}
	IRNode(IRType t, IRNode *n) {
		type = t;
		next = NULL;
		front = NULL;
	}
	
	IRNode(IRNode &ir) {
		for (int i = 0; i < 3; i++) {
			args[i] = ir.args[i];
		}
		type = ir.type;
		next = NULL;
		front = NULL;
	}

	void print() {
		switch (type) {
		case IRType::add:
			args[2].print();
			std::cout << ":= ";
			args[1].print(); 
			std::cout << "+ ";
			args[0].print();
			break;
		case IRType::sub:
			args[2].print();
			std::cout << ":= ";
			args[1].print();
			std::cout << "- ";
			args[0].print();
			break;
		case IRType::mult:
			args[2].print();
			std::cout << ":= ";
			args[1].print();
			std::cout << "* ";
			args[0].print();
			break;
		case IRType::div:
			args[2].print();
			std::cout << ":= ";
			args[1].print();
			std::cout << "/ ";
			args[0].print();
			break;
		case IRType::assign:
			args[1].print();
			std::cout << ":= ";
			args[0].print();
			break;
			
		case IRType::ret:
			std::cout << "return ";
			args[0].print();
			break;
		case IRType::func:
			std::cout << "function ";
			args[0].print();
			break;
		case IRType::jump:
			std::cout << "jmp ";
			args[0].print();
			break;
		case IRType::greater_jump:
			args[0].print();
			std::cout << "> ";
			args[1].print();
			std::cout << "goto ";
			args[2].print();
			break;
		case IRType::less_jump:
			args[0].print();
			std::cout << "< ";
			args[1].print();
			std::cout << "goto ";
			args[2].print();
			break;
		case IRType::equal_jump:
			args[0].print();
			std::cout << "== ";
			args[1].print();
			std::cout << "goto ";
			args[2].print();
			break;
		case IRType::unequal_jump:
			args[0].print();
			std::cout << "!= ";
			args[1].print();
			std::cout << "goto ";
			args[2].print();
			break;
		case IRType::ge_jump:
			args[0].print();
			std::cout << ">= ";
			args[1].print();
			std::cout << "goto ";
			args[2].print();
			break;
		case IRType::le_jump:
			args[0].print();
			std::cout << "<= ";
			args[1].print();
			std::cout << "goto ";
			args[2].print();
			break;
		case IRType::func_call:
			std::cout << "call ";
			args[0].print();
			std::cout << "to ";
			args[1].print();
			break;
		case IRType::func_param_in:
			std::cout << "param ";
			args[0].print();
			break;
		case IRType::print:
			std::cout << "print ";
			args[0].print();
			break;
		case IRType::input:
			std::cout << "in ";
			args[0].print();
			break;
		default:
			break;
		}
		std::cout << std::endl;
	}

	void print_for_json() {
		switch (type) {
		case IRType::add:
			args[2].print();
			std::cout << ":= ";
			args[1].print();
			std::cout << "+ ";
			args[0].print();
			break;
		case IRType::sub:
			args[2].print();
			std::cout << ":= ";
			args[1].print();
			std::cout << "- ";
			args[0].print();
			break;
		case IRType::mult:
			args[2].print();
			std::cout << ":= ";
			args[1].print();
			std::cout << "* ";
			args[0].print();
			break;
		case IRType::div:
			args[2].print();
			std::cout << ":= ";
			args[1].print();
			std::cout << "/ ";
			args[0].print();
			break;
		case IRType::assign:
			args[1].print();
			std::cout << ":= ";
			args[0].print();
			break;

		case IRType::ret:
			std::cout << "return ";
			args[0].print();
			break;
		case IRType::func:
			std::cout << "function ";
			args[0].print();
			break;
		case IRType::jump:
			std::cout << "jmp ";
			args[0].print();
			break;
		case IRType::greater_jump:
			args[0].print();
			std::cout << "> ";
			args[1].print();
			std::cout << "goto ";
			args[2].print();
			break;
		case IRType::less_jump:
			args[0].print();
			std::cout << "< ";
			args[1].print();
			std::cout << "goto ";
			args[2].print();
			break;
		case IRType::equal_jump:
			args[0].print();
			std::cout << "== ";
			args[1].print();
			std::cout << "goto ";
			args[2].print();
			break;
		case IRType::unequal_jump:
			args[0].print();
			std::cout << "!= ";
			args[1].print();
			std::cout << "goto ";
			args[2].print();
			break;
		case IRType::ge_jump:
			args[0].print();
			std::cout << ">= ";
			args[1].print();
			std::cout << "goto ";
			args[2].print();
			break;
		case IRType::le_jump:
			args[0].print();
			std::cout << "<= ";
			args[1].print();
			std::cout << "goto ";
			args[2].print();
			break;
		case IRType::func_call:
			std::cout << "call ";
			args[0].print();
			std::cout << "to ";
			args[1].print();
			break;
		case IRType::func_param_in:
			std::cout << "param ";
			args[0].print();
			break;
		case IRType::print:
			std::cout << "print ";
			args[0].print();
			break;
		case IRType::input:
			std::cout << "in ";
			args[0].print();
			break;
		default:
			break;
		}
	}

	void setArg(int i, IRArg *arg) {
		args[i].float_imm = arg->float_imm;
		args[i].int_imm = arg->int_imm;
		args[i].id = arg->id;
		//args[i]->temp_index = arg->temp_index;
		args[i].type = arg->type;
	}

	void setArg(int i, int int_imm) {
		args[i].int_imm = int_imm;
		args[i].type = IRAType::int_imm;
	}
	void setArg(int i, float float_imm) {
		args[i].float_imm = float_imm;
		args[i].type = IRAType::float_imm;
	}
	void setArg(int i, VarNode *var) {
		args[i].id = var;
		args[i].type = IRAType::var;
	}
	void setArg(int i, LabelNode *l) {
		args[i].label = l;
		args[i].type = IRAType::jump_label;
	}
	
	void setArg(int i, TempNode *n) {
		args[i].temp_index = n->index;
		args[i].temp = n;
		args[i].type = IRAType::temp;
	}
	void setArg(int i, FuncNode *f) {
		args[i].id = f;
		args[i].type = IRAType::func;
	}
	void setArg(int i, char c) {
		args[i].char_imm = c;
		args[i].type = IRAType::char_imm;
	}
	//
	static int index_assign_exp;
};


class IRCreator {
public:
	IRCreator();
	~IRCreator();
	void setSTM(STManager *s);

	void expState();
	// expect => IR
	void funcExpect();
	void meetRCB();
	void enter_scope() {
		stm->addTable(BlockType::white_block);
	}


	void print();
	void print_json();

	void clear_temp() {
		//temp_top_index = 0;
	}


	void new_sp() {
		ss_sp_stack[ss_sp_index++] = ss_index;
	}
	int sp_get(int i){ return ss_sp_stack[ss_sp_index - 1 - i]; }
	int sp_top() { return ss_sp_stack[ss_sp_index - 1]; }

	void pop_sp() { ss_sp_index--; }
	void ss_wrong_re() {
		for (int i = sp_top(); i < ss_len(); i++) {
			if (ss_stack[i] != NULL) {
				//std::cout << "delete" << (int)ss_stack[i]->type << std::endl ;
				delete ss_stack[i];
			}
				
		}
		int sp = ss_sp_stack[--ss_sp_index];
		ss_index = sp;
	}
	void ss_print() {
		for (int i = sp_top(); i < ss_len(); i++) {
			if (ss_stack[i] != NULL) {
				std::cout << "SS PRINT" << std::endl ;
				std::cout << ss_stack[i]->string_val << " " << (int)ss_stack[i]->type << std::endl;
			}

		}
	}



	void ss_push(SSNode *n) {
		ss_stack[ss_index++] = n;
	}
	int ss_len() { return ss_index; }
	SSNode *ss_top() { return ss_stack[ss_index - 1]; }
	void ss_pop() { 
		SSNode *d = ss_stack[ss_index-1];

		ss_stack[ss_index-1] = NULL;
		delete d;
		ss_index--;
	}
	SSNode *ss_get(int i) { return ss_stack[i]; }

	/*void jump_stack_push(IRNode *ti, IRNode *fi) {
		true_jump_stack[jump_stack_index] = ti;
		false_jump_stack[jump_stack_index] = fi;
		jump_stack_index++;
	}
	IRNode *js_true_get(int i) { return true_jump_stack[i]; }
	IRNode *js_false_get(int i) { return false_jump_stack[i]; }

	void js_new_sp() {
		js_sp_stack[js_sp_index++] = jump_stack_index;
	}
	void js_pop_sp() {
		js_sp_index--;
	}
	int js_sp_top() { return js_sp_stack[js_sp_index - 1]; }*/
	// parse error
	void handle_token_error(int line, Token t);

	// exp
	bool handle_assign_exp();
	bool handle_additive_exp();
	bool handle_multiplicative_exp();

	void handle_decl();
	void handle_idl();
	bool handle_init_declarator();

	bool handle_func_def();
	bool handle_func_call();
	//
	bool handle_return_state();
	bool handle_print_state();
	bool handle_in_state();
	bool handle_state();
	// logic
	bool handle_logic_exp();
	bool handle_logic_or_exp_1();
	bool handle_logic_or_exp_2();
	bool handle_logic_and_exp_1();
	bool handle_logic_and_exp_2();
	bool handle_rel_exp(Token op);

	bool handle_if_state_1();
	bool handle_if_state_2();
	bool handle_if_state_3();
	bool handle_if_state_4();

	bool handle_while_state();
	bool handle_while_state_1();
	bool handle_while_state_2();

	bool handle_for_state_1();
	bool handle_for_state_2();
	bool handle_for_state_3();

	bool is_parse_if = false;
	bool is_parse_else = false;
	bool is_parse_if_end = false;

	IRNode *rel_exp_first = NULL, *and_exp_first = NULL;
	bool expect_for_rel_exp = false, expect_for_and_exp = false;
	IRNode *for_exp_3_head = NULL, *for_exp_3_end = NULL;
	bool expect_for_exp_3 = false;

	LabelNode *_expect_true_label = NULL, *_expect_false_label = NULL, *_expect_end_label = NULL;
	LabelNode *_expect_while_to_logic_label = NULL;

	

	LabelNode *label_find(IRNode *ir) {
		std::map<IRNode *, LabelNode *>::const_iterator it;
		it = label_map.find(ir);

		if (it == label_map.end()) {
			return NULL;
		}
		else {
			return it->second;
		}
	}
	
	int ir_num = 0;

	IRNode *getIRHead() {
		return head;
	}

	void adjust();

private:
	IRNode *head, *cur;
	IRNode *gi_head = NULL, *gi_cur = NULL;
	int gi_num = 0;

	STManager *stm;
	ErrorManager *em;

	int temp_top_index = 0;// µ±Ç°temp

	std::vector<SSNode*> ss_stack;
	int ss_index = 0;
	std::vector<int> ss_sp_stack;
	int ss_sp_index = 0;

	std::map<IRNode*, LabelNode*> label_map;
	int label_index = 0;

	bool has_return = false;
	bool is_void_func = false;

	void addIRNode(IRNode *node);
	void addGlobalInit(IRNode *ir);

	void delete_ir_node(IRNode *node) {
		IRNode *fro = node->front, *next = node->next;
		if (fro != NULL) {
			fro->next = next;
		}
		else {
			head = next;
		}
		next->front = fro;
		delete node;
	}

	bool set_arg(IRNode *ir, int index, SSNode *n) {
		if (n->type == SSType::identifier) {
			VarNode *var = NULL;
			_handle_var_undecl(var, n);
			ir->setArg(index, var);
		}
		else if (n->type == SSType::float_const) {
			ir->setArg(index, n->float_val);
		}
		else if (n->type == SSType::int_const) {
			ir->setArg(index, n->int_val);
		}
		else if (n->type == SSType::temp_var) {
			//ir->setArg(index, TempType(n->int_val));
			ir->setArg(index, n->temp);
		}
		else if (n->type == SSType::char_const) {
			ir->setArg(index, n->char_val);
		}
		return true;
	}

	void label_finish(LabelNode *label) {
		IRNode *target = label->target;
		std::map<IRNode *, LabelNode *>::const_iterator it;
		it = label_map.find(target);

		if (it == label_map.end()) {
			label_map[target] = label;
			label->index = label_index++;
			target->label = label;
		}
		else {
			LabelNode *same_label = it->second;
			label->index = same_label->index;
		}
	}
	char *token_string[42] = {
		"int_const", "float_const", "char_const",
		"int_k", "float_k", "char_k", "bool_k", "void_k",
		"return_k", "for_k", "while_k", "if_k", "else_k", "print_k",
		"identifier",
		"","","","","",
		"LCB", "RCB", "LB", "RB", "LSB", "RSB", "dot", "semicolon", "comma",
		"multiply", "divide", "add", "substract",
		"logic_or", "logic_and",
		"double_equal", "unequal", "greater_equal", "less_equal", "greater", "less", "assign"
	};
};
