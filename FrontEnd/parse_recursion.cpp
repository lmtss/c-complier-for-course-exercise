#include "parse.h"
#include "ScopeExpect.h"
#include <iostream>
Token error_expect_token;
#define HE(s) if(s == false) { /*std::cout << "FUCK" << std::endl;*/ irc->ss_wrong_re(); return false;}
#define expect(t) if(  (error_expect_token = get_token()) != t) { /*std::cout << "expect token " << (int)t << " get " << (int)error_expect_token  << std::endl;*/ return false;}
#define expect_range(t1, t2)if(  (error_expect_token = get_token()) < t1 || error_expect_token > t2) { /*std::cout << "expect token " << (int)t1 << " to " << (int)t2 << " get " << (int)error_expect_token  << std::endl;*/ return false;}

void Parser::parse_translation_unit() {
	bool meetLCB = false, meetAssign = false, fin = false;
	int n = 0;
	while (!fin && n < 3) {
		n++;
		while (true) {
			Token ret = expect_token();
			if (ret >= Token::error) {
				fin = true;
				break;
			}
			else if (ret == Token::LCB && !meetAssign) {
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
		HE(parse_def_param_list());
		expect(Token::RB);
	}
	sp->meetFuncDef();
	HE(parse_compound_state());
	HE(irc->handle_func_def());
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
	
	Token ret = expect_token();
	// func
	if (ret == Token::LB) {
		SSNode *n = val();
		n->type = SSType::func_decl;
		irc->ss_push(n);

		get_token();
		if (expect_token() == Token::RB) {
			// func()
			get_token();
		}
		else {
			// func(p)
			expect_clear();
			HE(parse_decl_param_list());
			expect(Token::RB);
		}
	}
	else {
		irc->ss_push(val());
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

bool Parser::parse_decl_param_list() {

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
	//irc->ss_push(val());
	if (expect_token() == Token::identifier) {
		get_token();
		irc->ss_push(val());
	}
	expect_clear();
	irc->pop_sp();
	return true;
}

bool Parser::parse_def_param_list() {
	irc->new_sp();

	HE(parse_def_param());
	while (expect_token() == Token::comma) {
		get_token();
		HE(parse_def_param());
	}
	expect_clear();

	irc->pop_sp();

	return true;
}
bool Parser::parse_def_param() {
	irc->new_sp();
	HE(parse_type_spec());
	//irc->ss_push(val());
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
		get_token();
		HE(parse_return_state());
	}
	else if (t == Token::if_k) {
		expect_clear();
		HE(parse_if_state());
	}
	else if (t == Token::while_k) {
		get_token();
		HE(parse_while_state());
	}
	else if (t == Token::for_k) {
		get_token();
		HE(parse_for_state());
	}
	else {
		expect_clear();
		HE(parse_exp_state());
	}

	irc->pop_sp();

	return true;
}

bool Parser::parse_while_state() {
	irc->new_sp();
	expect(Token::LB);
	irc->handle_while_state_1();
	HE(parse_logic_or_exp());
	expect(Token::RB);
	irc->handle_while_state_2();

	HE(parse_state());
	HE(irc->handle_while_state());
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

bool Parser::parse_if_state() {
	// if
	irc->new_sp();
	expect(Token::if_k);
	expect(Token::LB);
	HE(parse_logic_or_exp());
	expect(Token::RB);
	irc->handle_if_state_3();
	
	HE(parse_state());

	if (expect_token() == Token::else_k) {
		get_token();
		irc->handle_if_state_1();
		HE(parse_state());
		irc->handle_if_state_2();
	}
	else {
		expect_clear();
		irc->handle_if_state_4();

	}
	
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
			//is_lb = true;
			/*get_token();
			HE(parse_assign_exp());
			expect(Token::RB);*/
			expect_clear();
			HE(parse_additive_exp());

			break;
		}
		else if (ret == Token::comma || ret == Token::semicolon || ret == Token::RB) {// assign_right
			expect_clear();
			//std::cout << "ASSR" << std::endl;
			HE(parse_additive_exp());

			break;
		}
		else if (ret == Token::assign && !is_lb) {// assign_left
			expect_clear();
			//std::cout << "ASSL" << std::endl;
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
	irc->new_sp();
	//std::cout << "POST EXP" << std::endl;
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
			//std::cout << "FC EXP" << std::endl;
			get_token();
			SSNode *n = val();
			n->type = SSType::func_call;
			irc->ss_push(n);
			get_token();//(
			//
			if (expect_token() != Token::RB) {
				HE(parse_call_arg_list()); //std::cout << "ARG L" << std::endl;
				get_token();
			}
			else {
				get_token();//)
			}
			//std::cout << "ARG L FIN" << std::endl;
			HE(irc->handle_func_call());
		}
		else {
			HE(parse_primary_exp(true));
		}
	}
	else {
		expect_clear();
		HE(parse_primary_exp(false));
	}
	irc->pop_sp();
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
bool Parser::parse_call_arg_list() {
	irc->new_sp();
	HE(parse_assign_exp());
	while (expect_token() == Token::comma) {
		get_token();
		HE(parse_assign_exp());
	}
	expect_clear();

	irc->pop_sp();
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

bool Parser::parse_logic_exp() {

	irc->new_sp();
	HE(irc->handle_logic_exp());
	HE(parse_logic_or_exp());
	irc->pop_sp();
	return true;
}

bool Parser::parse_logic_or_exp() {

	irc->new_sp();
	
	HE(parse_logic_and_exp());
	while (expect_token() == Token::logic_or) {
		get_token();
		HE(parse_logic_and_exp());
	}
	expect_clear();

	HE(irc->handle_logic_or_exp_2());
	irc->pop_sp();

	return true;
}

bool Parser::parse_logic_and_exp() {
	irc->new_sp();
	//HE(irc->handle_logic_and_exp_1());
	HE(parse_rel_exp());
	while (expect_token() == Token::logic_and) {
		get_token();
		HE(parse_rel_exp());
	}
	expect_clear();

	HE(irc->handle_logic_and_exp_2());
	irc->pop_sp();
	return true;
}

bool Parser::parse_rel_exp() {

	irc->new_sp();

	HE(parse_additive_exp());
	Token t;
	expect_range(Token::double_equal, Token::less);
	t = error_expect_token;
	//irc->ss_push(val());
	HE(parse_additive_exp());

	HE(irc->handle_rel_exp(t));

	irc->pop_sp();
	return true;
}

bool Parser::parse_for_state() {
	
	irc->new_sp();
	expect(Token::LB);

	if (expect_token() == Token::semicolon) {
		get_token();
	}
	else {
		expect_clear();
		HE(parse_exp());
		irc->ss_pop();
		expect(Token::semicolon);
	}

	irc->handle_while_state_1(); // to_logic_label 实际上可能指向state
	if (expect_token() == Token::semicolon) {
		get_token();
	}
	else {
		expect_clear();
		HE(parse_logic_or_exp());
		expect(Token::semicolon);
	}

	irc->handle_for_state_1();
	if (expect_token() == Token::RB) {
		get_token();
	}
	else {
		expect_clear();
		/*std::cout << (int)expect_token() << std::endl;
		expect_clear();*/
		HE(parse_assign_exp());
		irc->ss_pop();
		expect(Token::RB);
	}
	irc->handle_for_state_2();
	irc->handle_while_state_2();
	
	HE(parse_state());
	irc->handle_for_state_3();
	irc->handle_while_state();
	irc->pop_sp();
	return true;
}