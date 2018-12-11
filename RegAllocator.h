#pragma once
#include "FrontEnd\SymbolTable.h"
#include "FrontEndInterface.h"
#include <vector>
struct AGNode {
	AGNode *first = NULL;
};

struct AGEdge {
	AGEdge *next;
	int to;
};

struct VarAGNode : AGNode {
	VarNode *var = NULL;
};

struct TempAGNode : AGNode {
	TempNode *temp;
};

struct AllocGraph {
	std::vector<AGNode*> node_list;
	AllocGraph(int n) : node_list(n) {

	}
	void add_node(VarNode *var) {
		VarAGNode *n = new VarAGNode;
		n->var = var;
		node_list.push_back(n);
	}
	void add_node(TempNode *var) {
		TempAGNode *n = new TempAGNode;
		n->temp = var;
		node_list.push_back(n);
	}
	void add_edge(int from, int to) {

	}
};

class RegAllocator {
public:
	RegAllocator(FrontEndInterface *f);
	~RegAllocator();
	void alloc(int start_index, int end_index);
	VarNode *s(int i) { return sregs[i]; }
	TempNode *t(int i) { return tregs[i]; }
	void s(VarNode* var, int i = 7) { sregs[i] = var; }
private:
	void discover_live_ranges(int start_index, int end_index);
	void sb_alloc(int start_index, int end_index);
	void sb_linear_scan_alloc(int start_index, int end_index);
	void temp_lsa(int start_index, int end_index);

	AllocGraph *var_graph = NULL, *temp_graph = NULL;
	int var_live_range_num = 0;
	int temp_live_range_num = 0;
	FrontEndInterface *FEI;
	VarNode *sregs[8];
	TempNode *tregs[10];
	
};

