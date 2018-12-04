#pragma once
#include "SymbolTable.h"
#include <iostream>
#include <string>
enum ErrorType {
	undeclared, redeclared, param_num, no_ret
};

struct ErrorNode {
	FuncNode *func;
	int line;
	ErrorType type;
	ErrorNode *next;

	ErrorNode(FuncNode *f, int l, ErrorType t) {
		func = f;
		line = l;
		next = NULL;
		type = t;
	}
	void virtual print() = 0;
};

struct UndeclaredError : ErrorNode {
	string varName;
	UndeclaredError(FuncNode *f, int l, string s) : ErrorNode(f, l, ErrorType::undeclared) {
		varName = s;
	}
	void print() {
		std::cout << "In function \'" << func->name << "\':" << std::endl;
		std::cout << "line " << line << " error: \'" << varName << "\' undeclared" << std::endl;
	}
};

struct RedeclaredError : ErrorNode {
	string varName;
	RedeclaredError(FuncNode *f, int l, string s) : ErrorNode(f, l, ErrorType::redeclared) {
		varName = s;
	}
	void print() {
		std::cout << "In function \'" << func->name << "\':" << std::endl;
		std::cout << "line " << line << " error: \'" << varName << "\' redeclared" << std::endl;
	}
};

struct ParamNumError : ErrorNode {
	string funcName;
	int wrong_num, correct_num;
	ParamNumError(int l, string s, int w, int c) : ErrorNode(NULL, l, ErrorType::param_num) {
		funcName = s;
		wrong_num = w;
		correct_num = c;
	}
	void print() {
		std::cout << "argument num " << "pass " << wrong_num << " arg, should pass " << correct_num << " arg" << std::endl;
	}
};

struct NoRetError : ErrorNode {
	NoRetError(int l) : ErrorNode(NULL, l, ErrorType::no_ret) {

	}
	void print() {
		std::cout << "no return" << std::endl;
	}
};

class ErrorManager {
public:
	ErrorManager() {
		head = cur = NULL;
	}
	~ErrorManager() {

	}

	void addEN(FuncNode *f, int l, string s, ErrorType t) {
		if (t == ErrorType::undeclared) {
			addEN(new UndeclaredError(f, l, s));
		}
		else if (t == ErrorType::redeclared) {
			addEN(new RedeclaredError(f, l, s));
		}
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