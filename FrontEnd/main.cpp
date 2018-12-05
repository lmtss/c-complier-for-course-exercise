#include "lex.h"
#include "token.h"
#include "parse.h"
#include "ScopeExpect.h"
#include "SymbolTable.h"
#include "IRCreator.h"
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

int main() {
	yytext = new char[100];
	yytext[0] = '\0';

	FILE *fp = fopen("C:/Users/Lenovo-/Desktop/tt.c", "r");

	lex = new Lex();
	lex->setFP(fp);

	scopePredictor = new ScopeExpect();
	irCreator = new IRCreator();
	stManager = new STManager();
	irCreator->setSTM(stManager);
	scopePredictor->setIRC(irCreator);
	
	Parser *parser = new Parser();
	parser->lex = [] {return lex->lex(); };
	parser->set(scopePredictor, irCreator);

	parser->parse();

	irCreator->print();

	FrontEndInterface *FEI = new FrontEndInterface(irCreator);

	system("pause");
	return 0;
}