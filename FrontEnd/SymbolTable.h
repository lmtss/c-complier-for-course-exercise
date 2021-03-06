#pragma once
#include <string>
#include <vector>
#include <map>
#include <functional>
#include<iostream>

using namespace std;

class SymbolTable;


enum BlockType {
	func_block, switch_block, if_block, while_block, for_block, white_block, whole_block
};

enum SymbolType {
	var_t, func_t
};

struct TypeNode {
	
	string name;
	int len;
	int index;
};

struct IDNode {
	string name;
	SymbolType type;

	int declPosLine = -1;//����λ��

};

struct TempNode {

	int index;

	int reg_index = -1;
	int stack_address = -1;
	bool has_reg = false;
	bool at_reg = false;

	bool flag_live_scan = false;

	int live_start = 0;
	int live_end = 0;

	TempNode(int i) {
		index = i;	
	}

};

struct VarNode : IDNode {
	TypeNode *varType;
	int level = -1;//scope�㼶

	int reg_index = -1;
	int stack_address = -1;
	bool has_reg = false;
	bool at_reg = true;

	bool flag_live_scan = false;
	bool flag_global_scan = false;

	int arg_index = -1;
	bool is_arg = false;

	bool is_useless = true;

};

struct FuncNode : IDNode {
	bool isDefinied = false;
	TypeNode *retType;
	int param_num;
	vector<VarNode*> paraList;
	SymbolTable *table;
	vector<TempNode*> need_store_temp_list;

	//int cur_address = 0;
	int size = 0;
	int max_arg_size = 0;
	int var_num = 0;

	void addTemp(TempNode *temp) {
		need_store_temp_list.push_back(temp);
		temp->stack_address = size;
		size += 4;
	}
};

class SymbolTable {
public:
	SymbolTable(BlockType t, SymbolTable *f);
	~SymbolTable();
	VarNode* find(string& str) const;
	void insert(string& name, VarNode *id);
	void traverse(function<void(VarNode*)> func) {
		map<string, VarNode*>::iterator it;
		it = table.begin();
		while (it != table.end()) {
			func(it->second);
			it++;
		}
	}
	void back_traverse(function<void(VarNode*)> func) {
		traverse(func);
		SymbolTable *scope = father;
		while (scope->father != NULL) {
			scope->traverse(func);
			scope = scope->father;
		}
	}

	SymbolTable *father;
	BlockType type;
	map<string, VarNode*> table;
private:
	/*int cur_address = 0;
	SymbolTable *func = NULL;*/
};

class STManager {
public:
	STManager();
	~STManager();
	void addTable(BlockType t);
	SymbolTable* addFunc(FuncNode *func);
	void exitScope();

	VarNode* find(string& str) const;
	VarNode* findCurTable(string& str) const;
	FuncNode* findFunc(string& str) const;
	VarNode* insert(string& name, int level, TypeNode *type, int line);
	void insert(string& name, int level, int len, int line);

	void insert(int i) {
		temp_list.push_back(new TempNode(i));
	}
	void insert(TempNode *temp) {
		temp_list.push_back(temp);
	}
	//FuncNode* insertFunc(string& name, )
	void insertFunc(FuncNode *f);

	void func_traverse(function<void(VarNode*)> func) {
		map<string, FuncNode*>::iterator it;
		it = funcTable.begin();
		
		while (it != funcTable.end()) {
			cout << it->second->name << endl;
			it->second->table->traverse(func);
			it++;
		}
	}

	TypeNode *getBasicType(int i);

	int getCurLevel() {return curLevel;}
	BlockType getCurBlockType() { return curTable->type; }
	FuncNode *getCurFunc() { return curFunc; }
	SymbolTable *getWholeTable() { return wholeTable; }
	SymbolTable *getCurTable() { return curTable; }

	vector<TempNode*> temp_list;
private:
	SymbolTable *wholeTable;
	SymbolTable *curTable;
	TypeNode *basicTypes[4];
	FuncNode *curFunc;

	int curLevel;
	int curGlobalIndex;

	int cur_type_index = 4;
	
	map<string, FuncNode*> funcTable;

	TypeNode bTypes[4] = {
		{"int", 4, 0},{ "float", 0, 1},{ "void", 0 , 2},{ "", 0 , 3}
	};

};