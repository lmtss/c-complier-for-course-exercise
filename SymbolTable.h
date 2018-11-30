#pragma once
#include <string>
#include <vector>
#include <map>

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
};

struct IDNode {
	string name;
	SymbolType type;

	int declPosLine = -1;//ÉùÃ÷Î»ÖÃ

};

struct VarNode : IDNode {
	TypeNode *varType;
	int level = -1;//scope²ã¼¶
	int globalIndex;
};

struct FuncNode : IDNode {
	bool isDefinied = false;
	TypeNode *retType;
	vector<VarNode> paraList;
	SymbolTable *table;
};

class SymbolTable {
public:
	SymbolTable(BlockType t, SymbolTable *f);
	~SymbolTable();
	VarNode* find(string& str) const;
	void insert(string& name, VarNode *id);

	SymbolTable *father;
	BlockType type;
private:
	map<string, VarNode*> table;
	
};

class STManager {
public:
	STManager();
	~STManager();
	void addTable(BlockType t);
	void addFunc(FuncNode *func);
	void exitScope();

	VarNode* find(string& str) const;
	VarNode* findCurTable(string& str) const;
	FuncNode* findFunc(string& str) const;
	VarNode* insert(string& name, int level, TypeNode *type, int line);

	TypeNode *getBasicType(int i);

	int getCurLevel() {return curLevel;}
	BlockType getCurBlockType() { return curTable->type; }
	FuncNode *getCurFunc() { return curFunc; }
private:
	SymbolTable *wholeTable;
	SymbolTable *curTable;
	TypeNode *basicTypes[4];
	FuncNode *curFunc;

	int curLevel;
	int curGlobalIndex;

	map<string, FuncNode*> funcTable;

	TypeNode bTypes[4] = {
		{"int", 0},{ "float", 0 },{ "int", 0 },{ "int", 0 }
	};

};

