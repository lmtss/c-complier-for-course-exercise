#include "FrontEnd/lex.h"
#include "FrontEnd/token.h"
#include "FrontEnd/parse.h"
#include "FrontEnd/ScopeExpect.h"
#include "FrontEnd/SymbolTable.h"
#include "FrontEnd/IRCreator.h"
#include "FrontEndInterface.h"
#include <iostream>
#include <cstdio>
#include <type_traits>

char *yytext;
Lex *lex;
ScopeExpect *scopePredictor;
STManager* stManager;
IRCreator *irCreator;
SSNode *lexVal;
int yylineno;

int main(int argc, char *argv[]) {

	yytext = new char[100];
	yytext[0] = '\0';

	FILE *fp = NULL;

	bool is_print_to_json = false;

	if (argc > 1) {
		fp = fopen(argv[1], "r");
		if (argc > 2 && argv[2][0] == 't') {
			is_print_to_json = true;
		}
	}
	else {
		//std::cout << "SSSSSSSSSSSSS";
		fp = fopen("C:/Users/Lenovo-/Desktop/tt.c", "r");
	}

	lex = new Lex();
	lex->setFP(fp);
	/*while (true) {
		Token t = lex->lex();
		if (t >= Token::error)
			break;
	}
	lex->print();*/

	scopePredictor = new ScopeExpect();
	irCreator = new IRCreator();
	stManager = new STManager();
	irCreator->setSTM(stManager);
	scopePredictor->setIRC(irCreator);
	
	Parser *parser = new Parser();
	parser->lex = [] {return lex->lex(); };
	parser->set(scopePredictor, irCreator);

	parser->parse();

	if (is_print_to_json) {
		std::cout << "{" << std::endl;
		lex->print_for_json();
		irCreator->print_json();
		std::cout << "}" << std::endl;
	}
	else {
		irCreator->print();
	}
	

	FrontEndInterface *FEI = new FrontEndInterface(irCreator);

	//system("pause");
	return 0;
}