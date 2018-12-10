#pragma once
#include "SymbolTable.h"
#include <iostream>
#include <string>
enum ErrorType {
	undeclared, redeclared, param_num, no_ret, token
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
	void virtual print_for_json() = 0;
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
	void print_for_json() {
		std::cout << "\"func\":\"" << func->name << "\"," << std::endl;
		std::cout << "\"line\":" << line << "," << std::endl;
		std::cout << "\"content\":\"" << "error: " << varName << " undeclared" << "\"" << std::endl;
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
	void print_for_json() {
		std::cout << "\"func\":\"" << func->name << "\"," << std::endl;
		std::cout << "\"line\":" << line << "," << std::endl;
		std::cout << "\"content\":\"" << varName << " redeclared" << "\"" << std::endl;
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
	void print_for_json() {
		std::cout << "\"func\":\"" << "null" << "\"," << std::endl;
		std::cout << "\"content\":\"" << "argument num " << "pass " << wrong_num << " arg, should pass " << correct_num << " arg" << "\"" << std::endl;
	}
};

struct NoRetError : ErrorNode {
	NoRetError(FuncNode *f) : ErrorNode(f, 0, ErrorType::no_ret) {

	}
	void print() {
		std::cout << "In function \'" << func->name << "\':" << std::endl;
		std::cout << "no return" << std::endl;
	}
	void print_for_json() {
		std::cout << "\"func\":\"" << func->name << "\"," << std::endl;
		std::cout << "\"content\":\"" << "no return" << "\"" << std::endl;
	}
};

struct TokenError : ErrorNode {
	char *token;
	TokenError(FuncNode *f, int l, char *t) : ErrorNode(f, l, ErrorType::token) {
		token = t;
	}
	void print() {
		if(func != NULL)
			std::cout << "In function \'" << func->name << "\':" << std::endl;

		std::cout << "expect token " << token << std::endl;
	}
	void print_for_json() {
		if(func != NULL)
			std::cout << "\"func\":\"" << func->name << "\"," << std::endl;
		else
			std::cout << "\"func\":\"" << "whole_scope" << "\"," << std::endl;
		std::cout << "\"line\":" << line << "," << std::endl;
		std::cout << "\"content\":\"expect token " << token << " redeclared" << "\"" << std::endl;
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
		//e->print();
		has_error = true;
	}

	void print() {
		ErrorNode *node = head;
		while (node != NULL) {
			node->print();
			node = node->next;
		}
	}
	void print_for_json() {
		ErrorNode *node = head;
		while (node != NULL) {
			std::cout << "{" << std::endl;
			
			node->print_for_json();
			
			if (node->next != NULL)std::cout << "}," << std::endl;
			else std::cout << "}" << std::endl;
			node = node->next;
		}
	}
	bool hasError() { return has_error; }
private:
	ErrorNode *head, *cur;
	bool has_error = false;
};