#include "parse.h"
#include<iostream>


Parser::Parser() {
}

Parser::~Parser() {
}

void Parser::parse() {
	parse_translation_unit();
}

Token Parser::get_token() {
	Token ret;
	if (_token_pool.empty()) {
		ret = lex();
		lex_val = lexVal;
		if(is_val_token(ret))
			cur_line = lex_val->code_line;
		if (ret == Token::END) {

		}
		else if (ret == Token::error) {

		}
	}
	else {
		ret = _token_pool.front();
		lex_val = _token_pool.val();
		if (is_val_token(ret))
			cur_line = lex_val->code_line;
		_token_pool.pop();
	}
	//std::cout << "get_token " << (int)ret << std::endl;
	return ret;
}


Token Parser::expect_token() {
	Token ret;
	if (_token_pool.expect_end()) {
		ret = lex();
		if (ret != Token::END && ret != Token::error)
			_token_pool.push(ret, lexVal);
	}
	
	return _token_pool.expect_token();
}
/*
void Parser::expect(Token t) {
	if (get_token() != t) {
		std::cout << "ERROR" << std::endl;
	}
}*/