#include "SymbolTable.h"
#include<iostream>
SymbolTable::SymbolTable(BlockType t, SymbolTable *f) {
	type = t;
	father = f;
}

SymbolTable::~SymbolTable() {
}

VarNode* SymbolTable::find(std::string& str) const{
	std::map<string, VarNode*>::const_iterator it;
	it = table.find(str);

	if (it == table.end()) {
		return NULL;
	}
	else {
		return it->second;
	}
}


//插入普通变量
void SymbolTable::insert(std::string& str, VarNode *id) {

	table[str] = id;
}




STManager::STManager() {
	wholeTable = new SymbolTable(BlockType::whole_block, NULL);
	curTable = wholeTable;
	curLevel = 0;
	curGlobalIndex = 0;
	curFunc = NULL;


	// init basic type
	basicTypes[0] = new TypeNode;
	basicTypes[1] = new TypeNode;
	basicTypes[2] = new TypeNode;
	basicTypes[3] = new TypeNode;

	basicTypes[0]->name = "int";
	basicTypes[1]->name = "float";
	basicTypes[2]->name = "char";
	basicTypes[3]->name = "void";

	/*bTypes[0] = new TypeNode;
	bTypes[1] = new TypeNode;
	bTypes[2] = new TypeNode;
	bTypes[3] = new TypeNode;*/

	/*bTypes[0].name = "int";
	bTypes[1].name = "float";
	bTypes[2].name = "char";
	bTypes[3].name = "void";*/

}

STManager::~STManager() {
}

void STManager::addTable(BlockType t) {
	SymbolTable *table = new SymbolTable(t, curTable);
	curTable = table;

	curLevel++;
}

void STManager::addFunc(FuncNode *func) {

	if (curTable != wholeTable) {
		// error
	}
	
	SymbolTable *table = new SymbolTable(func_block, wholeTable);
	func->table = table;
	curTable = table;

	curFunc = func;

	curLevel++;
}

void STManager::exitScope() {
	curTable = curTable->father;
	curLevel--;
}

TypeNode *STManager::getBasicType(int i) {
	TypeNode *node = bTypes + i;
	return node;
}

VarNode *STManager::find(string& str) const {
	VarNode *res = NULL;
	SymbolTable *table = curTable;
	
	do {
		res = table->find(str);
		if (res != NULL) {
			return res;
		}
		else 
			table = table->father;
		
	} while (table != wholeTable);

	return NULL;
}

VarNode *STManager::findCurTable(string& str) const {
	return curTable->find(str);
}

FuncNode* STManager::findFunc(string& str) const {
	std::map<string, FuncNode*>::const_iterator it;
	it = funcTable.find(str);

	if (it == funcTable.end()) {
		return NULL;
	}
	else {
		return it->second;
	}
}

VarNode* STManager::insert(string& name, int level, TypeNode *type, int line) {
	VarNode* id = new VarNode;
	id->name = name;
	id->varType = type;
	id->level = level;
	id->declPosLine = line;
	id->globalIndex = curGlobalIndex;

	curTable->insert(name, id);

	curGlobalIndex++;

	return id;
}