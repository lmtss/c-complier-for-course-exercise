#pragma once
#include "RegAllocator.h"
#include "FrontEndInterface.h"
#include <fstream>
#include <string>

class ASMCreator {
public:
	ASMCreator(FrontEndInterface *f, RegAllocator *r, fstream &o);
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
	void create_return();
	//bool createing_call = false;
	int now_a_reg_index = 0;
	IRNode *creating_call_ir = NULL;
	FuncNode *creating_def_func = NULL;

	int load_store(IRNode *ir, int i);
	std::string &to_arg_str(IRNode *ir, int i);
	void handle_t9(int imm);
	FrontEndInterface *FEI;
	RegAllocator *alloc;
	FILE *save_fp = NULL;
	fstream &out;

	char *jump_strs[6];
};

