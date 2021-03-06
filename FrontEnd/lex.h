#pragma once
#include "token.h"
#include "ScopeExpect.h"
#include "SS.h"
#include<fstream>
#include<string>
#include <vector>

extern char *yytext;
extern ScopeExpect *scopePredictor;
extern STManager* stManager;
extern IRCreator *irCreator;
//extern ASTNode *lexVal;
extern SSNode *lexVal;

extern int yylineno;

enum class LexState {
	Start = 0,
	ID,
	digit_1, digit_2,
	int_1, int_2, int_3,
	if_2,
	char_1, char_2, char_3, char_4,
	float_1, float_2, float_3, float_4, float_5,
	for_2, for_3,
	void_1, void_2, void_3, void_4,
	return_1, return_2, return_3, return_4, return_5, return_6,
	while_1, while_2, while_3, while_4, while_5,
	bool_1, bool_2, bool_3, bool_4,
	else_1, else_2, else_3, else_4,
	print_1, print_2, print_3, print_4, print_5,
	char_const_1, char_const_2, char_const_3,
	equal_1,
	greater_1,
	less_1,
	logic_or_1,
	logic_and_1,
	d_1, d_2, d_3, d_4,

	// fin
	int_const = 100, float_const, char_const,
	int_k, float_k, char_k, bool_k, void_k,
	return_k, for_k, while_k, if_k, else_k, print_k, in_k,
	identifier,
	
	//120 { } ( ) . ; , 
	LCB = 120, RCB, LB, RB, LSB, RSB, dot, semicolon, comma,  
	// * / + -
	multiply = 129, divide, add, substract,
	logic_or, logic_and,
	
	// == >= <= > < =
	double_equal, unequal,greater_equal, less_equal, greater, less, assign,
	
	
	
	//error
	error = 200, empty_char_error
	
};

class Lex {
public:
	Lex();
	~Lex();
	Token lex();
	void setFP(FILE *fp);
	void print();
	void print_for_json();
private:
	FILE *fp;
	LexState curState;
	

	int bufPos;
	bool needBack;
	char charBack;

	LexState table[70][127];
	void(*func[70])();
	std::vector<Token> token_list;

	void initTable();
	void initFunc();
	int getC();

	
	void _setKTI(LexState s, LexState i);
	void _setK(LexState k_1, std::string s, LexState k);
	void _meetLetter(LexState s, LexState i);
	void _meetDigit(LexState s, LexState i);
};