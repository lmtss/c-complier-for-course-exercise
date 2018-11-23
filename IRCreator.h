#pragma once
#include "AST.h"
#include "SymbolTable.h"
#include "Error.h"
#include <iostream>
struct IRNode {
	IRNode *next;

	void virtual print() = 0;
};

struct FuncIR : IRNode {
	FuncNode *func;

	void print() {
		std::cout << "Function  " << func->name << "  ret " << func->retType->name << std::endl;
	}
};

struct AssignIR : IRNode {
	int varIndex_1, varIndex_2;
	bool isTemp;

	void print() {
		if (isTemp) {
			std::cout << "var" << varIndex_1 << " := " << "temp" << varIndex_2 << std::endl;
		}
		else {
			std::cout << "var" << varIndex_1 << " := " << "var" << varIndex_2 << std::endl;
		}
	}
};

struct AssignImmIR : IRNode {
	bool isTemp;
	int varIndex;
	string imm;

	void print() {
		if (isTemp) {
			std::cout << "temp" << varIndex << " := " << imm << std::endl;
		}
		else {
			std::cout << "var" << varIndex << " := " << imm << std::endl;
		}
	}
};

struct RetIR : IRNode {
	bool isTemp;
	int varIndex;

	void print() {
		if (!isTemp) 
			std::cout << "Return  var" << varIndex << std::endl;
		else 
			std::cout << "Return  temp" << varIndex << std::endl;
	}
};

class IRCreator {
public:
	IRCreator();
	~IRCreator();
	void setSTM(STManager *s);
	// ASTNode => IR
	void decl(ASTNode *declNode);
	void expState(ASTNode *expStateNode);
	void handleExpression(ASTNode *exp);
	// expect => IR
	void funcExpect(ASTNode *typeSpecNode, ASTNode *identifier, ASTNode *declaratorNode);
	void meetRCB();

	void print();
	
private:
	IRNode *head, *cur;

	STManager *stm;
	ErrorManager *em;

	void addIRNode(IRNode *node);

	// exp
	
	TypeNode* handleAssign(ASTNode *assign);

	// decl
	void handleIDL(TypeNode *type, ASTNode *idl);
	void handleInitDeclarator(TypeNode *type, ASTNode *node);
};
