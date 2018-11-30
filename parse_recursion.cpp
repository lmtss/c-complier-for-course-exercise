#include "parse.h"
#include "ScopeExpect.h"
#include <iostream>
Token error_expect_token;
#define HE(s) if(s == false) { std::cout << "FUCK" << std::endl; irc->ss_wrong_re(); return false;}
#define expect(t) if(  (error_expect_token = get_token()) != t) { std::cout << "expect token " << (int)t << " get " << (int)error_expect_token  << std::endl; return false;}

void Parser::parse_translation_unit() {
	bool meetLCB = false, meetAssign = false;
	int n = 0;
	while (n < 3) {
		n++;
		while (true) {
			Token ret = expect_token();

			if (ret == Token::LCB && !meetAssign) {
				expect_clear();
				//std::cout << "func_def ";
				parse_func_def();
				break;
			}
			else if (!meetLCB && (ret == Token::assign || ret == Token::semicolon)) {
				expect_clear();
				//std::cout << "decl ";
				parse_decl();
				break;
			}
			meetLCB = ret == Token::LCB || meetLCB;
			meetAssign = (ret == Token::assign || ret == Token::semicolon) || meetAssign;
		}
		meetAssign = meetLCB = false;
	}
}

bool Parser::parse_func_def() {

	irc->new_sp();

	HE(parse_type_spec());
	expect(Token::identifier);

	irc->ss_push(val());
	
	expect(Token::LB);
	
	Token ret = expect_token();
	expect_clear();
	if (ret == Token::RB) {
		get_token();
	}
	else {
		HE(parse_param_list());
		expect(Token::RB);
	}
	sp->meetFuncDef();
	HE(parse_compound_state());
	irc->handle_func_def();
	irc->pop_sp();
	
	return true;
}

bool Parser::parse_decl() {

	irc->new_sp();

	HE(parse_type_spec());
	HE(parse_init_declarator_list());
	expect(Token::semicolon);
	irc->handle_decl();
	irc->pop_sp();

	return true;
}

bool Parser::parse_init_declarator_list() {

	irc->new_sp();

	HE(parse_init_declarator()); 
	while (expect_token() == Token::comma) {
		get_token();
		HE(parse_init_declarator());
	}
	expect_clear();

	irc->pop_sp();

	return true;
}
bool Parser::parse_init_declarator() {

	irc->new_sp();

	HE(parse_declarator());
	if (expect_token() == Token::assign) {

		get_token();
		HE(parse_initializer());
	}
	else {
		expect_clear();
	}

	HE(irc->handle_init_declarator());

	irc->clear_temp();

	irc->pop_sp();

	return true;
}
bool Parser::parse_declarator() {

	irc->new_sp();

	expect(Token::identifier);
	irc->ss_push(val());
	Token ret = expect_token();
	// func
	if (ret == Token::LB) {
		get_token();
		if (expect_token() == Token::RB) {
			// func()
			get_token();
		}
		else {
			// func(p)
			expect_clear();
			HE(parse_param_list());
			expect(Token::RB);
		}
	}
	else {
		expect_clear();
	}
		
	irc->pop_sp();

	return true;
}
bool Parser::parse_initializer() {
	irc->new_sp();
	
	HE(parse_assign_exp());
	irc->pop_sp();

	return true;
}

bool Parser::parse_type_spec() {
	Token ret = expect_token();
	//std::cout << (int)ret << std::endl;
	if (ret == Token::int_k) {
		get_token();
		
	}
	else if(ret == Token::float_k){
		get_token();
	}
	else {
		return false;
	}
	irc->ss_push(val());

	return true;
}

bool Parser::parse_param_list() {

	irc->new_sp();

	HE(parse_param_decl());
	while (expect_token() == Token::comma) {
		get_token();
		HE(parse_param_decl());
	}
	expect_clear();

	irc->pop_sp();

	return true;
}

bool Parser::parse_param_decl() {
	irc->new_sp();
	HE(parse_type_spec());
	expect(Token::identifier);
	irc->ss_push(val());
	irc->pop_sp();
	return true;
}

bool Parser::parse_compound_state() {
	irc->new_sp();
	expect(Token::LCB);
	sp->meetCurlyBrace();
	Token t;
	while ((t = expect_token()) != Token::RCB) {
		//expect_clear();
		
		// block_item
		if (t == Token::int_k || t == Token::float_k) {// decl
			expect_clear();
			parse_decl();
		}
		else {// state
			expect_clear();
			parse_state();
		}
	}
	
	expect(Token::RCB);
	irc->meetRCB();
	irc->pop_sp();
	return true;
}

bool Parser::parse_state() {

	irc->new_sp();

	Token t = expect_token();

	if (t == Token::LCB) {
		expect_clear();
		HE(parse_compound_state());
	}
	else if (t == Token::return_k) {
		std::cout << "asdasd";
		get_token();
		HE(parse_return_state());
	}
	else {
		expect_clear();
		HE(parse_exp_state());
	}

	irc->pop_sp();

	return true;
}

bool Parser::parse_return_state() {
	irc->new_sp();
	HE(parse_assign_exp());
	expect(Token::semicolon);
	HE(irc->handle_return_state());

	irc->pop_sp();
	return true;
}

bool Parser::parse_exp_state() {
	if (expect_token() == Token::semicolon) {
		get_token();
	}
	else {

		irc->new_sp();

		expect_clear();
		HE(parse_exp());
		expect(Token::semicolon);

		irc->pop_sp();
	}
	return true;
}

bool Parser::parse_exp() {

	irc->new_sp();

	HE(parse_assign_exp());
	irc->clear_temp();
	while (expect_token() == Token::comma) {
		get_token();
		HE(parse_assign_exp());

		irc->ss_pop();
		irc->clear_temp();
	}
	expect_clear();

	irc->pop_sp();
	return true;
}

bool Parser::parse_assign_exp() {

	irc->new_sp();

	bool is_lb = false;
	Token ret;
	while (true) {
		ret = expect_token();
		if (ret == Token::LB) {
			is_lb = true;
		}
		else if (ret == Token::RB) {
			is_lb = false;
		}
		else if (ret == Token::comma || ret == Token::semicolon) {// assign_right
			expect_clear();

			HE(parse_additive_exp());

			break;
		}
		else if (ret == Token::assign && !is_lb) {// assign_left
			expect_clear();
			HE(parse_assign_left());

			expect(Token::assign);
		}
	}
	
	

	HE(irc->handle_assign_exp());

	irc->pop_sp();

	return true;
}

bool Parser::parse_additive_exp() {

	irc->new_sp();

	if (expect_token() == Token::substract) {
		get_token();
		irc->ss_push(val());
	}
	else
		expect_clear();

	HE(parse_multiplicative_exp());

	Token t;
	while ((t = expect_token()) == Token::add || t == Token::substract) {
		get_token();

		irc->ss_push(val());

		HE(parse_multiplicative_exp());
	}

	expect_clear();

	HE(irc->handle_additive_exp());
	irc->pop_sp();

	return true;
}
bool Parser::parse_multiplicative_exp() {

	irc->new_sp();

	HE(parse_postfix_exp());

	Token t;
	while ((t = expect_token()) == Token::multiply || t == Token::divide) {
		get_token();

		irc->ss_push(val());

		HE(parse_postfix_exp());

	}

	HE(irc->handle_multiplicative_exp());

	expect_clear();

	irc->pop_sp();

	return true;
}
bool Parser::parse_postfix_exp() {
	Token t0 = expect_token();
	if (t0 == Token::identifier) {
		Token t1 = expect_token();
		if (t1 == Token::LSB) {
			get_token();
			get_token();//[
			//
			HE(parse_const_exp());
			expect(Token::RSB);

		}
		else if (t1 == Token::LB) {
			get_token();
			get_token();//(
			//
			if (expect_token() != Token::RB) {

			}
			else {
				get_token();//)

			}
		}
		else {
			HE(parse_primary_exp(true));
		}
	}
	else {
		std::cout << " P2" << std::endl;
		expect_clear();
		HE(parse_primary_exp(false));
	}
	return true;
}
bool Parser::parse_assign_left() {
	Token t0 = expect_token();
	expect(Token::identifier);
	
	//ASTNode *id = lex_val;
	if (expect_token() == Token::LSB) {
		get_token();
		//irc->ss_push(val());
		//id
		get_token();//[
					//
		HE(parse_const_exp());
		expect(Token::RSB);

	}
	else {

		SSNode *id = val();
		irc->ss_push(id);

		expect_clear();
	}

	return true;
}

bool Parser::parse_primary_exp(bool isID) {
	irc->new_sp();
	if (isID) {
		get_token();
		irc->ss_push(val());
	}
	else {
		if (expect_token() == Token::LB) {
			get_token();
			std::cout << "AAAAAAAAAAAA" << std::endl;
			HE(parse_assign_exp());
			expect(Token::RB);
		}
		else {
			expect_clear();
			HE(parse_const());
		}
		
	}
	irc->pop_sp();
	return true;
}

bool Parser::parse_const() {
	Token t = get_token();
	if (t == Token::int_const) {

		irc->ss_push(val());

	}
	else if (t == Token::float_const) {
		irc->ss_push(val());
	}
	else {
		return false;
	}
	return true;
}

bool Parser::parse_const_exp() {
	return true;
}