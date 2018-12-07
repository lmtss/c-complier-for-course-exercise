#include "lex.h"
#include "token.h"
#include<cstdio>
#include<iostream>
Lex::Lex() {
	initTable();
	initFunc();

	curState = LexState::Start;
	bufPos = 0;
	needBack = false;
	yylineno = 0;
}

Lex::~Lex() {
}

void Lex::setFP(FILE *f) {
	fp = f;
}

void Lex::initTable() {
	// Start
	for (int i = 'a'; i <= 'z'; i++)
		table[0][i] = LexState::ID;
	for (int i = 'A'; i <= 'Z'; i++)
		table[0][i] = LexState::ID;
	for (int i = '0'; i <= '9'; i++)
		table[0][i] = LexState::digit_1;

	table[0]['i'] = LexState::int_1;
	table[0]['c'] = LexState::char_1;
	table[0]['f'] = LexState::float_1;
	table[0]['r'] = LexState::return_1;
	table[0]['b'] = LexState::bool_1;
	table[0]['w'] = LexState::while_1;
	table[0]['v'] = LexState::void_1;
	table[0]['e'] = LexState::else_1;

	table[0]['\''] = LexState::char_const_1;

	table[0][' '] = LexState::Start;

	table[0]['{'] = LexState::LCB;
	table[0]['}'] = LexState::RCB;
	table[0]['('] = LexState::LB;
	table[0][')'] = LexState::RB;
	table[0]['['] = LexState::LSB;
	table[0][']'] = LexState::RSB;
	table[0]['.'] = LexState::dot;
	table[0][';'] = LexState::semicolon;
	table[0][','] = LexState::comma;
	table[0]['='] = LexState::equal_1;

	table[0]['>'] = LexState::greater_1;
	table[0]['<'] = LexState::less_1;

	table[0]['&'] = LexState::logic_and_1;
	table[0]['|'] = LexState::logic_or_1;

	table[0]['*'] = LexState::multiply;
	table[0]['/'] = LexState::d_1;
	//table[0]['/'] = LexState::divide;
	table[0]['+'] = LexState::add;
	table[0]['-'] = LexState::substract;

	// key
	for (int j = (int)LexState::int_1; j < (int)LexState::else_4; j++) {
		for (int i = 'a'; i <= 'z'; i++)
			table[j][i] = LexState::ID;
		for (int i = 'A'; i <= 'Z'; i++)
			table[j][i] = LexState::ID;
		for (int i = '0'; i <= '9'; i++)
			table[j][i] = LexState::ID;
		for (int i = ' '; i <= '/'; i++)
			table[j][i] = LexState::identifier;
		for (int i = ':'; i <= '@'; i++)
			table[j][i] = LexState::identifier;
		for (int i = '['; i <= '`'; i++)
			table[j][i] = LexState::identifier;
		for (int i = '{'; i <= '~'; i++)
			table[j][i] = LexState::identifier;
	}

	// int_k

	_setK(LexState::int_1, "nt", LexState::int_k);

	// float_k

	_setK(LexState::float_1, "loat", LexState::float_k);

	// char_k
	_setK(LexState::char_1, "har", LexState::char_k);
	// bool_k
	_setK(LexState::bool_1, "ool", LexState::bool_k);

	_setK(LexState::return_1, "eturn", LexState::return_k);

	_setK(LexState::while_1, "hile", LexState::while_k);

	_setK(LexState::void_1, "oid", LexState::void_k);

	_setK(LexState::else_1, "lse", LexState::else_k);

	table[(int)LexState::int_1]['f'] = LexState::if_2;
	_setKTI(LexState::if_2, LexState::if_k);
	table[(int)LexState::float_1]['o'] = LexState::for_2;
	table[(int)LexState::for_2]['r'] = LexState::for_3;
	_setKTI(LexState::for_3, LexState::for_k);

	//
	_setKTI(LexState::ID, LexState::identifier);
	_meetLetter(LexState::ID, LexState::ID);
	_meetDigit(LexState::ID, LexState::ID);
	//

	_meetDigit(LexState::digit_1, LexState::digit_1);
	_meetDigit(LexState::digit_2, LexState::digit_2);
	_meetLetter(LexState::digit_1, LexState::error);
	_meetLetter(LexState::digit_2, LexState::error);
	_setKTI(LexState::digit_1, LexState::int_const);
	table[(int)LexState::digit_1]['.'] = LexState::digit_2;
	_setKTI(LexState::digit_2, LexState::float_const);

	// =
	
	for (int i = 0; i < 127; i++) {
		table[(int)LexState::equal_1][i] = LexState::assign;
		table[(int)LexState::less_1][i] = LexState::less;
		table[(int)LexState::greater_1][i] = LexState::greater;
	}
		
	table[(int)LexState::equal_1]['='] = LexState::double_equal;
	table[(int)LexState::less_1]['='] = LexState::less_equal;
	table[(int)LexState::greater_1]['='] = LexState::greater_equal;

	table[(int)LexState::logic_and_1]['&'] = LexState::logic_and;
	table[(int)LexState::logic_or_1]['|'] = LexState::logic_or;

	//
	
	//table[(int)LexState::d_2]['\n'] = LexState::Start;
	for (int i = 0; i < 127; i++) {
		table[(int)LexState::d_1][i] = LexState::divide;
		table[(int)LexState::d_2][i] = LexState::d_2;
		table[(int)LexState::d_4][i] = LexState::d_3;
		table[(int)LexState::d_3][i] = LexState::d_3;
	}
	table[(int)LexState::d_1]['/'] = LexState::d_2;
	table[(int)LexState::d_1]['*'] = LexState::d_3;
	table[(int)LexState::d_3]['*'] = LexState::d_4;
	table[(int)LexState::d_4]['/'] = LexState::Start;
}

void Lex::initFunc() {
	void(*f)() = [] {};
	for (int i = 0; i < 50; i++)
		func[i] = f;

	//
	//func[Token::int_const] = [] {printf("INTC"); };
	func[Token::LCB] = [] {};
	func[Token::RCB] = [] {  };
	func[Token::add] = [] {lexVal = new SSNode(SSType::add, yylineno, yytext); };
	func[Token::substract] = [] {lexVal = new SSNode(SSType::sub, yylineno, yytext); };
	func[Token::multiply] = [] {lexVal = new SSNode(SSType::mult, yylineno, yytext); };
	func[Token::divide] = [] {lexVal = new SSNode(SSType::div, yylineno, yytext); };
	//func[Token::su] = [] {};
	func[Token::identifier] = [] {lexVal = new SSNode(SSType::identifier, yylineno, yytext); };

	func[Token::int_k] = [] {lexVal = new SSNode(SSType::int_k, yylineno, yytext); };
	func[Token::float_k] = [] {lexVal = new SSNode(SSType::float_k, yylineno, yytext); };
	func[Token::void_k] = [] {lexVal = new SSNode(SSType::void_k, yylineno, yytext); };

	func[Token::else_k] = [] { /*lexVal = new SSNode(SSType::else_k, yylineno, yytext);*/ };
	func[Token::int_const] = [] {lexVal = new SSNode(SSType::int_const, yylineno, yytext); };
	func[Token::float_const] = [] {lexVal = new SSNode(SSType::float_const, yylineno, yytext); };

}

void Lex::_meetDigit(LexState s, LexState f) {
	for (int i = '0'; i <= '9'; i++)
		table[(int)s][i] = f;
}
void Lex::_meetLetter(LexState s, LexState f) {
	for (int i = 'a'; i <= 'z'; i++)
		table[(int)s][i] = f;
	for (int i = 'A'; i <= 'Z'; i++)
		table[(int)s][i] = f;
}
void Lex::_setKTI(LexState s, LexState f) {
	for (int i = ' '; i <= '/'; i++)
		table[(int)s][i] = f;
	for (int i = ':'; i <= '@'; i++)
		table[(int)s][i] = f;
	for (int i = '['; i <= '`'; i++)
		table[(int)s][i] = f;
	for (int i = '{'; i <= '~'; i++)
		table[(int)s][i] = f;
}
void Lex::_setK(LexState k_1, std::string s, LexState k) {
	int len = s.length();
	for (int i = 0; i < len; i++) {
		table[(int)k_1 + i][s[i]] = (LexState)((int)k_1 + i + 1);
	}
	_setKTI((LexState)((int)k_1 + len), (LexState)(int)k);
}
Token Lex::lex() {

	char c;
	curState = LexState::Start;
	bufPos = 0;
	//int fff = 0;
	while ((c = getC()) != EOF) {
		//std::cout << fff++;
		if (c == '\n') {
			yylineno++;
			if (curState == LexState::d_2) {
				curState = LexState::Start;
			}
			continue;
		}
		if (c == '\t')
			c = ' ';
		LexState res = table[(int)curState][c];
		if ((int)res >= 100 && res < LexState::error) {
			if ((int)res < 120 || res >= LexState::greater || res == LexState::divide) {
				needBack = true;
				charBack = c;
				yytext[bufPos] = '\0';
			}
			else {
				yytext[bufPos] = c;
				yytext[bufPos + 1] = '\0';
			}
			
			//printf("%s\n", yytext);

			Token t = (Token)((int)res - 100);
			token_list.push_back(t);
			//std::cout << (int)t;
			func[t]();

			return t;
		}
		else {
			if (c != ' ') {
				yytext[bufPos] = c;
				bufPos++;
			}
			
			curState = res;
		}
		
	}
	return Token::END;
}
int Lex::getC() {
	if (needBack) {
		needBack = false;
		return charBack;
	}
	else
		return fgetc(fp);
}

void Lex::print() {
	int id_index = 1, int_const_index = 1, float_const_index = 1, char_const_index = 1;

	for (int i = 0; i < token_list.size(); i++) {
		Token t = token_list[i];
		if (t == Token::identifier) {
			std::cout << "<" << "id, " << id_index++ << ">";
		}
		else if (t == Token::int_const) {
			std::cout << "<" << "int_const, " << int_const_index++ << ">";
		}
		else if (t == Token::float_const) {
			std::cout << "<" << "float_const, " << float_const_index++ << ">";
		}
		else if (t >= Token::int_k && t <= Token::else_k) {
			std::cout << "<" << "k, " << (t - Token::int_k + 1) << ">";
		}
		else if (t >= Token::LCB && t <= Token::assign) {
			std::cout << "<" << "p, " << (t - Token::LCB + 1) << ">";
		}
		else if (t == Token::char_const) {
			std::cout << "<" << "char_const, " << char_const_index++ << ">";
		}
		//std::cout << std::endl;
	}
}

void Lex::print_for_json() {
	int id_index = 0, int_const_index = 0, float_const_index = 0, char_const_index = 0;
	std::cout << "\"Token\":" << "\"";
	for (int i = 0; i < token_list.size(); i++) {
		Token t = token_list[i];
		if (t == Token::identifier) {
			std::cout << "<" << "id, " << id_index++ << ">";
		}
		else if (t == Token::int_const) {
			std::cout << "<" << "int_const, " << int_const_index++ << ">";
		}
		else if (t == Token::float_const) {
			std::cout << "<" << "float_const, " << float_const_index++ << ">";
		}
		else if (t >= Token::int_k && t <= Token::else_k) {
			std::cout << "<" << "k, " << (t - Token::int_k + 1) << ">";
		}
		else if (t >= Token::LCB && t <= Token::assign) {
			std::cout << "<" << "p, " << (t - Token::LCB + 1) << ">";
		}
		else if (t == Token::char_const) {
			std::cout << "<" << "char_const, " << char_const_index++ << ">";
		}
	}
	std::cout << "\",";
}