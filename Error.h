#pragma once
#include "SymbolTable.h"
#include <iostream>
#include <string>
enum ErrorType {
	undeclared
};

struct ErrorNode {
	FuncNode *func;
	int line;
	ErrorNode *next;

	ErrorNode(FuncNode *f, int l) {
		func = f;
		line = l;
		next = NULL;
	}
	void virtual print() = 0;
};

struct UndeclaredError : ErrorNode {
	string varName;
	UndeclaredError(FuncNode *f, int l, string s) : ErrorNode(f, l) {
		varName = s;
	}
	void print() {
		std::cout << "In function \'" << func->name << "\':" << std::endl;
		std::cout << "line " << line << " error: \'" << varName << "\' undeclared" << std::endl;
	}
};

struct RedeclaredError : ErrorNode {
	string varName;
	RedeclaredError(FuncNode *f, int l, string s) : ErrorNode(f, l) {
		varName = s;
	}
	void print() {
		std::cout << "In function \'" << func->name << "\':" << std::endl;
		std::cout << "line " << line << " error: \'" << varName << "\' redeclared" << std::endl;
	}
};

class ErrorManager {
public:
	ErrorManager() {
		head = cur = NULL;
	}
	~ErrorManager() {

	}

	void addEN(ErrorNode *e) {
		if (head == NULL) {
			head = e;
			cur = e;
		}
		else {
			cur->next = e;
			cur = e;
		}
		e->print();
		
	}

	void print() {
		ErrorNode *node = head;
		while (node != NULL) {
			node->print();
			node = node->next;
		}
	}

private:
	ErrorNode *head, *cur;
};

