#pragma once
#include "token.h"
#include <queue>
#include "AST.h"
#include "ScopeExpect.h"
#include "IRCreator.h"

extern ASTNode *lexVal;

struct TokenPool {
	int head, end, expect;
	Token tokens[100];
	ASTNode *astNodes[100];
	TokenPool() {
		head = end = expect = 0;
	}
	void push(Token t, ASTNode *node) {
		tokens[end] = t;
		astNodes[end] = node;
		end = (end + 1) % 99;
	}
	void pop() {
		head = (head + 1) % 99;
		expect = head;
	}
	Token front() {
		return tokens[head];
	}
	bool empty() {
		return head == end;
	}
	void clear() {
		expect = head;
	}
	Token expect_token() {
		//expect++;
		return tokens[expect++];
	}
	bool expect_end() {
		return expect == end;
	}
	ASTNode *val() {
		return astNodes[head];
	}
};

class Parser {
public:
	Parser();
	~Parser();

	void parse();
	void set(ScopeExpect *scopePredictor, IRCreator *irCreator) {
		sp = scopePredictor;
		irc = irCreator;
	}


	Token(*lex)();
private:
	Token get_token();
	// 
	Token expect_token();
	void expect_clear() {_token_pool.clear();}
	//
	//void expect(Token t);

	// parse
	void parse_translation_unit();
	bool parse_func_def();
	bool parse_decl();
	bool parse_type_spec();
	bool parse_param_list();
	bool parse_param_decl();
	bool parse_compound_state();
	bool parse_state();
	bool parse_init_declarator_list();
	bool parse_init_declarator();
	bool parse_declarator();
	bool parse_initializer();

	bool parse_assign_exp();
	bool parse_exp_state();

	bool parse_exp();

	bool parse_assign_left();
	bool parse_additive_exp();
	bool parse_multiplicative_exp();
	bool parse_postfix_exp();
	bool parse_primary_exp(bool isID);
	bool parse_const();
	bool parse_const_exp();

	TokenPool _token_pool;
	
	ASTNode *$$, *$1, *$2, *$3, *$4, *$5;
	ASTNode *lex_val;

	ScopeExpect *sp;
	IRCreator *irc;
};