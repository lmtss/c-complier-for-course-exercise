#pragma once
#include <cstdarg>
#include "SS.h"
#include "SymbolTable.h"
#include "Error.h"
#include "token.h"
#include <iostream>
#include <vector>

#define _handle_var_undecl(vptr, sptr) if((vptr = stm->find(sptr->string_val)) == NULL){ em->addEN(stm->getCurFunc(), sptr->code_line, sptr->string_val, ErrorType::undeclared); return false;}

enum class IRAType {
	int_imm, float_imm, var, temp, NONE
};
enum class IRType {
	add, sub, mult, div, assign, ret, func
};

struct IRArg {
	int int_imm;
	float float_imm;
	int temp_index;
	IDNode *id;
	IRAType type = IRAType::NONE;
	void print() {
		switch (type) {
		case IRAType::int_imm:
			std::cout << int_imm;
			break;
		case IRAType::float_imm:
			std::cout << float_imm;
			break;
		case IRAType::var:
			std::cout << id->name;
			break;
		case IRAType::temp:
			std::cout << 't' << temp_index;
			break;
		default:
			break;
		}
		std::cout << " ";
	}
};

struct IRNode {
	IRArg args[3];
	IRType type;
	IRNode *next, *front;
	IRNode() {
		type = IRType::assign;
		next = NULL;
		front = NULL;
	}
	IRNode(IRType t, ...) {
		type = t;
		va_list arg_list;
		va_start(arg_list, t);
		if (type >= IRType::add && type <= IRType::div) {

			for (int i = 0; i < 3; i++) {
				IRAType arg_type = va_arg(arg_list, IRAType);

				args[i].type = arg_type;

				switch (arg_type) {
				case IRAType::float_imm:
					args[i].float_imm = (float)va_arg(arg_list, double); break;
				case IRAType::int_imm:
					args[i].int_imm = va_arg(arg_list, int); break;
				case IRAType::temp:
					args[i].temp_index = va_arg(arg_list, int); break;
				case IRAType::var:
					args[i].id = va_arg(arg_list, VarNode*); break;
				default:
					break;
				}

			}
		}
		else if (type == IRType::assign) {

			for (int i = 1; i < 3; i++) {
				IRAType arg_type = va_arg(arg_list, IRAType);

				args[i].type = arg_type;
				switch (arg_type) {
				case IRAType::float_imm:
					args[i].float_imm = (float)va_arg(arg_list, double); break;
				case IRAType::int_imm:
					args[i].int_imm = va_arg(arg_list, int); break;
				case IRAType::temp:
					args[i].temp_index = va_arg(arg_list, int); break;
				case IRAType::var:
					args[i].id = va_arg(arg_list, VarNode*); break;
				default:
					break;
				}
				//std::cout << (int)arg_type << " " << args[i].float_imm << "WTFFFFFFFFF" << std::endl;
				//args[i].print();
			}
			va_end(arg_list);
		}
		else if (type == IRType::func) {

			args[2].id = va_arg(arg_list, FuncNode*);
		}
		else if (type == IRType::ret) {
			IRAType arg_type = va_arg(arg_list, IRAType);

			args[2].type = arg_type;

			switch (arg_type) {
			case IRAType::float_imm:
				args[2].float_imm = (float)va_arg(arg_list, double); break;
			case IRAType::int_imm:
				args[2].int_imm = va_arg(arg_list, int); break;
			case IRAType::temp:
				args[2].temp_index = va_arg(arg_list, int); break;
			case IRAType::var:
				args[2].id = va_arg(arg_list, VarNode*); break;
			default:
				break;
			}
		}

		
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
			args[2].print();
			std::cout << ":= ";
			args[1].print();
			break;
			
		case IRType::ret:
			std::cout << "return ";
			args[2].print();
			break;
		case IRType::func:
			std::cout << "function ";
			args[2].print();
			break;
		default:
			break;
		}
		std::cout << std::endl;
	}
	void setArg(int i, IRArg *arg) {
		args[i].float_imm = arg->float_imm;
		args[i].int_imm = arg->int_imm;
		args[i].id = arg->id;
		//args[i]->temp_index = arg->temp_index;
		args[i].type = arg->type;
	}
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


	void print();

	void clear_temp() {
		temp_top_index = 0;
	}


	void new_sp() {
		ss_sp_stack[ss_sp_index++] = ss_index;
	}

	int sp_top() { return ss_sp_stack[ss_sp_index - 1]; }

	void pop_sp() { ss_sp_index--; }
	void ss_wrong_re() {
		for (int i = sp_top(); i < ss_len(); i++) {
			if (ss_stack[i] != NULL)
				delete ss_stack[i];
		}
		int sp = ss_sp_stack[--ss_sp_index];
		ss_index = sp;
	}



	void ss_push(SSNode *n) {
		ss_stack[ss_index++] = n;
	}
	int ss_len() { return ss_index; }
	SSNode *ss_top() { return ss_stack[ss_index - 1]; }
	void ss_pop() { 
		SSNode *d = ss_stack[ss_index];

		ss_stack[ss_index] = NULL;
		delete d;
		ss_index--;
	}
	SSNode *ss_get(int i) { return ss_stack[i]; }


	// exp
	bool handle_assign_exp();
	bool handle_additive_exp();
	bool handle_multiplicative_exp();

	void handle_decl();
	void handle_idl();
	bool handle_init_declarator();

	void handle_func_def();
	//
	bool handle_return_state();
	
private:
	IRNode *head, *cur;

	STManager *stm;
	ErrorManager *em;

	int temp_top_index = 0;// µ±Ç°temp

	std::vector<SSNode*> ss_stack;
	int ss_index = 0;
	std::vector<int> ss_sp_stack;
	int ss_sp_index = 0;

	void addIRNode(IRNode *node);
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
};
