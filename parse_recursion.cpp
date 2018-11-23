#include "parse.h"
#include "AST.h"
#include "ScopeExpect.h"
#include <iostream>

#define HE(s) if(s == false) { std::cout << "FUCK" << std::endl; return false;}
#define expect(t) if(get_token() != t) { std::cout << "FUCK " << (int)t << std::endl; return false;}

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
	HE(parse_type_spec());
	$1 = $$;
	
	Token ret;
	expect(Token::identifier);
	$2 = lex_val;
	
	expect(Token::LB);
	
	ret = expect_token();
	expect_clear();
	if (ret == Token::RB) {
		get_token();
		sp->meetFuncDef($1, $2, NULL);
	}
	else {
		HE(parse_param_list());
		$3 = $$;
		expect(Token::RB);
		sp->meetFuncDef($1, $2, $3);
	}
	HE(parse_compound_state());
	
	return true;
}

bool Parser::parse_decl() {
	HE(parse_type_spec());
	$1 = $$;
	HE(parse_init_declarator_list());
	$2 = $$;
	expect(Token::semicolon);
	$$ = new ASTNode(ASTType::AST_decl, 2, $1, $2);
	irc->decl($$);
	return true;
}

bool Parser::parse_init_declarator_list() {
	HE(parse_init_declarator()); 
	ASTNode *node = new ASTNode(ASTType::AST_init_declarator_list, 1, $$);
	while (expect_token() == Token::comma) {
		get_token();
		HE(parse_init_declarator());
		node->addChild($$);
	}
	expect_clear();
	$$ = node;

	return true;
}
bool Parser::parse_init_declarator() {
	HE(parse_declarator());
	$1 = $$;
	if (expect_token() == Token::assign) {

		get_token();
		HE(parse_initializer());
		$$ = new ASTNode(ASTType::AST_init_declarator, 2, $1, $$);
	}
	else {
		expect_clear();
		$$ = new ASTNode(ASTType::AST_init_declarator, 1, $1);
	}

	return true;
}
bool Parser::parse_declarator() {
	expect(Token::identifier);
	$1 = lex_val;
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
		$$ = new ASTNode(ASTType::AST_declarator, 1, $1);
	}
		
	return true;
}
bool Parser::parse_initializer() {
	
	HE(parse_assign_exp());
	$$ = new ASTNode(ASTType::AST_initializer, 1, $$);
	return true;
}

bool Parser::parse_type_spec() {
	Token ret = expect_token();
	//std::cout << (int)ret << std::endl;
	if (ret == Token::int_k) {
		get_token();
		$$ = new ASTNode(ASTType::AST_type_spec, 1, lex_val);
		
	}
	else if(ret == Token::float_k){
		get_token();
		$$ = new ASTNode(ASTType::AST_type_spec, 1, lex_val);
	}
	else {
		return false;
	}

	return true;
}

bool Parser::parse_param_list() {
	HE(parse_param_decl());
	ASTNode *node = new ASTNode(ASTType::AST_init_declarator_list, 1, $$);
	while (expect_token() == Token::comma) {
		get_token();
		HE(parse_param_decl());
		node->addChild($$);
	}
	expect_clear();
	$$ = node;

	return true;
}

bool Parser::parse_param_decl() {
	HE(parse_type_spec());
	expect(Token::identifier);
	return true;
}

bool Parser::parse_compound_state() {
	expect(Token::LCB);
	sp->meetCurlyBrace();
	Token t;
	while ((t = expect_token()) != Token::RCB) {
		//expect_clear();
		
		// block_item
		if (expect_token() == Token::identifier) {// decl
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
	return true;
}

bool Parser::parse_state() {
	if (expect_token() == Token::LCB) {
		expect_clear();
		HE(parse_compound_state());
	}
	else {
		expect_clear();
		HE(parse_exp_state());
	}

	return true;
}

bool Parser::parse_exp_state() {
	if (expect_token() == Token::semicolon) {
		get_token();
	}
	else {
		expect_clear();
		HE(parse_exp());
		irc->handleExpression($$);
	}
	return true;
}

bool Parser::parse_exp() {
	HE(parse_assign_exp());
	ASTNode *node = new ASTNode(ASTType::AST_exp, 1, $$);
	while (expect_token() == Token::comma) {
		get_token();
		HE(parse_assign_exp());
		node->addChild($$);
	}
	expect_clear();

	$$ = node;
	return true;
}

bool Parser::parse_assign_exp() {
	bool meetAssign = false, meetCS = false;
	while (true) {
		Token ret = expect_token();

		if (ret == Token::assign && !meetCS) {// left
			expect_clear();
			HE(parse_assign_left());
			ASTNode *n = $$;
			expect(Token::assign);
			HE(parse_assign_exp());
			$$ = new ASTNode(ASTType::AST_assignment_exp, 3, n, NULL, $$);
			break;
		}
		else if ((ret == Token::comma || ret == Token::semicolon) && !meetAssign) {// right
			expect_clear();
			HE(parse_additive_exp());
			break;
		}

		meetAssign = ret == Token::assign || meetAssign;
		meetCS = ret == Token::comma || ret == Token::semicolon || meetCS;

	}

	/*Token t0 = expect_token(); 
	if (t0 != Token::identifier) {
		expect_clear();
		HE(parse_primary_exp(false));
		$$ = new ASTNode(ASTType::AST_assignment_exp, 1, $$);
	}
	else {
		Token t1 = expect_token();
		if (t1 == Token::assign) {
			//expect_clear();
			get_token();
			ASTNode *n = lex_val;
			
			get_token();
			HE(parse_assign_exp());
			$$ = new ASTNode(ASTType::AST_assignment_exp, 3, n, NULL, $$);
			//std::cout << "AE " << $1->content << " " << $$->content << std::endl;
		}
		else {
			expect_clear();
			HE(parse_primary_exp(true));
			$$ = new ASTNode(ASTType::AST_assignment_exp, 1, $$);
		}
	}*/
	
	return true;
}

bool Parser::parse_additive_exp() {
	HE(parse_multiplicative_exp());
	ASTNode *node = new ASTNode(ASTType::AST_init_declarator_list, 1, $$);
	Token t;
	while ((t = expect_token()) == Token::add || t == Token::substract) {
		get_token();
		//ASTNode *t = lex_val;
		node->addChild(lex_val);
		HE(parse_multiplicative_exp());
		node->addChild($$);
	}
	expect_clear();
	$$ = node;
	return true;
}
bool Parser::parse_multiplicative_exp() {
	HE(parse_postfix_exp());
	ASTNode *node = new ASTNode(ASTType::AST_init_declarator_list, 1, $$);
	Token t;
	while ((t = expect_token()) == Token::multiply || t == Token::divide) {
		get_token();
		//ASTNode *t = lex_val;
		node->addChild(lex_val);
		HE(parse_postfix_exp());
		node->addChild($$);
	}
	expect_clear();
	$$ = node;
	return true;
}
bool Parser::parse_postfix_exp() {
	Token t0 = expect_token();
	if (t0 == Token::identifier) {
		Token t1 = expect_token();
		if (t1 == Token::LSB) {
			get_token();
			ASTNode *id = lex_val;//id
			get_token();//[
			//
			HE(parse_const_exp());
			expect(Token::RSB);

			$$ = new ASTNode(ASTType::AST_postfix_exp, 2, id, $$);
		}
		else if (t1 == Token::LB) {
			get_token();
			$1 = lex_val;//id
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
			$$ = new ASTNode(ASTType::AST_postfix_exp, 1, $$);
		}
	}
	else {
		HE(parse_primary_exp(false));
		$$ = new ASTNode(ASTType::AST_postfix_exp, 1, $$);
	}
	return true;
}
bool Parser::parse_assign_left() {
	Token t0 = expect_token();
	if (t0 == Token::identifier) {
		Token t1 = expect_token();
		if (t1 == Token::LSB) {
			get_token();
			ASTNode *id = lex_val;//id
			get_token();//[
						//
			HE(parse_const_exp());
			expect(Token::RSB);

			$$ = new ASTNode(ASTType::AST_assign_left, 2, id, $$);

		}
		else {
			HE(parse_primary_exp(true));
			$$ = new ASTNode(ASTType::AST_assign_left, 1, $$);
		}
	}
	else {
		HE(parse_primary_exp(false));
		$$ = new ASTNode(ASTType::AST_assign_left, 1, $$);
	}
	return true;
}

bool Parser::parse_primary_exp(bool isID) {
	if (isID) {
		get_token();
		//std::cout << "PE " << lex_val->content << std::endl;
		$$ = new ASTNode(ASTType::AST_primary_exp, 1, lex_val);
	}
	else {
		HE(parse_const());
		$$ = new ASTNode(ASTType::AST_primary_exp, 1, $$);
	}
	return true;
}

bool Parser::parse_const() {
	Token t = get_token();
	if (t == Token::int_const) {
		//std::cout << "int_const";
	}
	else if (t == Token::float_const) {

	}
	else {
		return false;
	}
	$$ = new ASTNode(ASTType::AST_const, 1, lex_val);
	return true;
}

bool Parser::parse_const_exp() {
	return true;
}