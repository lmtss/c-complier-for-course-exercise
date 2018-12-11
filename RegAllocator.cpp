#include "RegAllocator.h"
#include "FrontEnd\SymbolTable.h"
#include <queue>

#define _get_ir(i) FEI->getIR(i)

struct cmp {
	bool operator() (TempNode *&a, TempNode *&b) const {
		return a->live_end > b->live_end;
	}
};

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
						var->at_reg = false;
					}
					else {
						var->has_reg = true;
						var->at_reg = true;
						var->reg_index = var_live_range_num - 1 + 16;
						sregs[var_live_range_num - 1] = var;
					}
				}

			}
			/*else if (ir->args[i].type == IRAType::temp) {
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
			}*/

		}
	}
	temp_lsa(start_index, end_index);
}

void RegAllocator::sb_linear_scan_alloc(int start_index, int end_index) {
	for (int i = start_index; i < end_index; i++) {

	}
}

void RegAllocator::temp_lsa(int start_index, int end_index) {
	IRNode *ir = NULL;

	std::priority_queue<TempNode *, vector<TempNode *>, cmp> active;
	std::queue<TempNode *> unhandled;
	TempNode *current = NULL;

	for (int i = start_index; i < end_index; i++) {
		ir = FEI->getIR(i);
		if (ir->type >= IRType::add && ir->type <= IRType::div) {
			TempNode *temp = NULL;
			if (ir->args[2].type == IRAType::temp) {
				temp = ir->args[2].temp;
				if (temp->live_start == 0) {
					temp->live_start = i;
					unhandled.push(temp);
				}
					
			}
			for (int j = 0; j < 2; j++) {// use
				if (ir->args[j].type == IRAType::temp) {
					TempNode *t = ir->args[j].temp;
					if (t != temp) {
						if (t->live_end == 0)
							t->live_end = i;
					}
					
				}
			}
		}
		else if (ir->type == IRType::assign) {
			TempNode *temp = NULL;
			if (ir->args[1].type == IRAType::temp) {
				temp = ir->args[1].temp;
				if (temp->live_start == 0) {
					temp->live_start = i;
					unhandled.push(temp);
				}
					
			}
			if (ir->args[0].type == IRAType::temp) {
				temp = ir->args[0].temp;
				if (temp->live_end == 0)
					temp->live_end = i;
			}
		}
		else if (ir->type == IRType::func_param_in || ir->type == IRType::print) {
			if (ir->args[0].type == IRAType::temp) {
				TempNode *temp = ir->args[0].temp;
				//temp = ir->args[0].temp;
				if (temp->live_end == 0)
					temp->live_end = i;
			}
		}
		
	}


	TempNode *pq_fin = NULL;
	int cur_reg = 1;
	while (!unhandled.empty()) {
		current = unhandled.front();
		unhandled.pop();
		if (active.empty()) {
			current->at_reg = true;
			current->has_reg = true;
			current->reg_index = 8;
			tregs[0] = current;
			active.push(current);
			std::cout << "TREG " << current->index << " " << current->reg_index << " " << current->live_start << " " << current->live_end << std::endl;
		}
		else {
			TempNode *active_top = active.top();
			if (current->live_start >= active_top->live_end) {
				active.pop();
				current->at_reg = true;
				current->has_reg = true;
				current->reg_index = active_top->reg_index;
				active.push(current);

				std::cout << "TREG " << current->index << " " << current->reg_index << " " << current->live_start << " " << current->live_end << std::endl;
			}
			else {
				if (active_top->reg_index == 15) { // spill

				}
				else {
					
					current->at_reg = true;
					current->has_reg = true;
					current->reg_index = active_top->reg_index + cur_reg++;
					tregs[active_top->reg_index - 7] = current;
					active.push(current);
					std::cout << "TREG " << current->index << " " << current->reg_index << " " << current->live_start << " " << current->live_end << std::endl;
				}
			}
		}
	}
}