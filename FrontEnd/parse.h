#pragma once
#include "token.h"
#include <queue>
#include "SS.h"
#include "ScopeExpect.h"
#include "IRCreator.h"

extern SSNode *lexVal;

struct TokenPool {
	int head, end, expect;
	Token tokens[100];
	SSNode *astNodes[100];
	TokenPool() {
		head = end = expect = 0;
	}
	void push(Token t, SSNode *node) {
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
	SSNode *val() {
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

	SSNode *val() { return lex_val; }
	//
	//void expect(Token t);

	// parse
	void parse_translation_unit();
	bool parse_func_def();
	bool parse_decl();
	bool parse_type_spec();
	bool parse_decl_param_list();
	bool parse_param_decl();
	bool parse_def_param_list();
	bool parse_def_param();
	bool parse_compound_state();
	bool parse_state();
	bool parse_init_declarator_list();
	bool parse_init_declarator();
	bool parse_declarator();
	bool parse_initializer();

	bool parse_assign_exp();
	bool parse_exp_state();
	bool parse_return_state();
	bool parse_if_state();
	bool parse_while_state();
	bool parse_for_state();

	bool parse_exp();

	bool parse_call_arg_list();

	bool parse_assign_left();
	bool parse_additive_exp();
	bool parse_multiplicative_exp();
	bool parse_postfix_exp();
	bool parse_primary_exp(bool isID);
	bool parse_const();
	bool parse_const_exp();

	bool parse_logic_exp();
	bool parse_logic_or_exp();
	bool parse_logic_and_exp();
	//bool parse_equal_exp();
	bool parse_rel_exp();

	TokenPool _token_pool;
	
	SSNode *lex_val;

	ScopeExpect *sp;
	IRCreator *irc;
};