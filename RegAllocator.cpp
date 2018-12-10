#include "RegAllocator.h"
#include "FrontEnd\SymbolTable.h"
#define _get_ir(i) FEI->getIR(i)
RegAllocator::RegAllocator(FrontEndInterface *f) : FEI(f) {
	for (int i = 0; i < 8; i++) {
		sregs[i] = NULL;
	}
	for (int i = 0; i < 10; i++) {
		tregs[i] = NULL;
	}
}

RegAllocator::~RegAllocator() {
}

void RegAllocator::alloc(int start_index, int end_index) {
	var_live_range_num = temp_live_range_num = 0;
	/*var_graph = new AllocGraph(20);
	temp_graph = new AllocGraph(20);
	sb_alloc(start_index, end_index);*/
	/*temp_graph = new AllocGraph(20);
	var_graph = new AllocGraph(20);*/
	sb_alloc(start_index, end_index);
	
}

void RegAllocator::discover_live_ranges(int start_index, int end_index) {
	for (int i = start_index; i < end_index; i++) {
		IRNode *ir = _get_ir(i);
		for (int i = 0; i < 3; i++) {
			if (ir->args[i].type == IRAType::var) {
				VarNode *var = ir->args[i].getVar();
				if (!var->flag_live_scan) {
					var_live_range_num++;
					var->flag_live_scan = true;

				}
				
			}
			else if (ir->args[i].type == IRAType::temp) {
				//int t = ir->args[i].temp_index;
				TempNode *t = ir->args[i].temp;
				if (!t->flag_live_scan) {
					temp_live_range_num++;
					t->flag_live_scan = true;
				}
			}
		}
	}

	
	for (int i = start_index; i < end_index; i++) {
		IRNode *ir = _get_ir(i);
		for (int i = 0; i < 3; i++) {
			if (ir->args[i].type == IRAType::var) {
				VarNode *var = ir->args[i].getVar();
				if (!var->flag_live_scan) {
					var_live_range_num++;
				}

			}
			else if (ir->args[i].type == IRAType::temp) {
				//int t = ir->args[i].temp_index;
				TempNode *t = ir->args[i].temp;
				if (!t->flag_live_scan) {
					temp_live_range_num++;
				}
			}
		}
	}


}

void RegAllocator::sb_alloc(int start_index, int end_index) {
	for (int i = start_index; i < end_index; i++) {
		IRNode *ir = _get_ir(i);
		for (int i = 0; i < 3; i++) {
			if (ir->args[i].type == IRAType::var) {
				VarNode *var = ir->args[i].getVar();
				var->is_useless = false;
				if (var->is_arg && var->arg_index < 4) {
					var->reg_index = var->arg_index + 4;
					//std::cout << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAA " << var->name << std::endl;
					var->at_reg = true;
					var->has_reg = true;
				}
				else if (!var->flag_live_scan) {
					var_live_range_num++;
					var->flag_live_scan = true;
					if (var_live_range_num > 7) {
						var->has_reg = false;
					}
					else {
						var->has_reg = true;
						var->at_reg = true;
						var->reg_index = var_live_range_num - 1 + 16;
						sregs[var_live_range_num - 1] = var;
					}
				}

			}
			else if (ir->args[i].type == IRAType::temp) {
				TempNode *t = ir->args[i].temp;
				if (!t->flag_live_scan) {
					temp_live_range_num++;
					t->flag_live_scan = true;
					if (temp_live_range_num > 8) {
						t->has_reg = false;
					}
					else {
						t->has_reg = true;
						t->at_reg = true;
						t->reg_index = temp_live_range_num - 1 + 8;
						tregs[temp_live_range_num - 1] = t;
					}
				}
			}

		}
	}
}