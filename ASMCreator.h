#pragma once
#include "RegAllocator.h"
#include "FrontEndInterface.h"
#include <fstream>
#include <string>
#include <iostream>

enum class ASMOS {
	start, first, imm, sec
};

class ASMOut {
public:
	ASMOut(fstream &o, bool b);
	~ASMOut();
	friend ASMOut& operator<< (ASMOut &out, char *s);
	friend ASMOut& operator<< (ASMOut &out, int imm);
	friend ASMOut& operator<< (ASMOut &out, std::string &s);
	friend void operator<< (ASMOut &out, basic_ostream<char, char_traits<char>>& (*_Pfn)(basic_ostream<char, char_traits<char>>&));
	bool ir_end = false;
private:
	fstream &fout;
	bool is_print_json = false;
	int index = 0;
	ASMOS state;
	bool ls = false;
};


class ASMCreator {
public:
	ASMCreator(FrontEndInterface *f, RegAllocator *r, fstream &o, bool print_json);
	~ASMCreator();
	void create_head();
	void create_block(int start, int end);
private:

	void create_func(IRNode *ir);
	void create_assign(IRNode *ir);
	void create_asmd(IRNode *ir);
	void create_print(IRNode *ir);
	void create_jump(IRNode *ir);
	
	void create_param_in(IRNode *ir);
	void create_call(IRNode *ir);
	void create_call_fin();
	void create_return(IRNode *ir);
	//bool createing_call = false;
	int now_a_reg_index = 0;
	IRNode *creating_call_ir = NULL;
	FuncNode *creating_def_func = NULL;

	int load_store(IRNode *ir, int i);
	void store(VarNode *var);
	void handle_t9(int imm);
	int var_offset(VarNode *var);
	FrontEndInterface *FEI;
	RegAllocator *alloc;
	FILE *save_fp = NULL;
	//fstream &out;
	ASMOut out;
	char *help_buf = NULL;
	char *jump_strs[6] = {
		"beq", "bne", "bge", "ble", "bgt", "blt"
	};
	char *reg_strs[32] = {
		"$zero", "$at", "$v0", "$v1", "$a0", "$a1", "a2", "a3",
		"$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7",
		"$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7",
		"$t8", "$t9",
		"$k0", "$k1",
		"$gp",
		"$sp", "$fp",
		"$ra"
	};
};
