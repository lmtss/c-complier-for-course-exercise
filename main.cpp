#include "FrontEnd/lex.h"
#include "FrontEnd/token.h"
#include "FrontEnd/parse.h"
#include "FrontEnd/ScopeExpect.h"
#include "FrontEnd/SymbolTable.h"
#include "FrontEnd/IRCreator.h"
#include "FrontEndInterface.h"
#include "RegAllocator.h"
#include "ASMCreator.h"
#include <iostream>
#include <fstream>
#include <cstdio>
#include <type_traits>
#include "DAG.h"

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
	FILE *asm_save_fp = NULL;
	fstream out;
	bool is_print_to_json = false;

	if (argc > 1) {
		fp = fopen(argv[1], "r");
		if (argc > 2) {
			out.open(argv[2], ios::out);
			if (argc > 3 && argv[3][0] == 't') {
				is_print_to_json = true;
			}
		}
		else {
			out.open("C:/Users/Lenovo-/Desktop/asm.s", ios::out);
		}
	}
	else {
		fp = fopen("C:/Users/Lenovo-/Desktop/tt.c", "r");
		out.open("C:/Users/Lenovo-/Desktop/asm.s", ios::out);
	}

	lex = new Lex();
	lex->setFP(fp);
	/*while (true) {
		Token t = lex->lex();
		if (t >= Token::error)
			break;
	}*/
	//lex->print();

	scopePredictor = new ScopeExpect();
	irCreator = new IRCreator();
	stManager = new STManager();
	irCreator->setSTM(stManager);
	scopePredictor->setIRC(irCreator);
	
	Parser *parser = new Parser();
	parser->lex = [] {return lex->lex(); };
	parser->set(scopePredictor, irCreator);

	parser->parse();
	irCreator->adjust();

	if (is_print_to_json) {
		std::cout << "{" << std::endl;
		lex->print_for_json();
		irCreator->print_json();
		
	}
	else {
		irCreator->print();
	}
	

	FrontEndInterface *FEI = new FrontEndInterface(irCreator, stManager);
	DAG dag(FEI);
	dag.gene_blocks();
	dag.optimize();

	FEI->adjust();
	if (is_print_to_json)
		FEI->print_for_json();
	else
		FEI->print();
	RegAllocator *alloc = new RegAllocator(FEI);
	ASMCreator *ac = new ASMCreator(FEI, alloc, out, is_print_to_json);
	ac->create_head();


	if (is_print_to_json) {
		std::cout << std::endl;
		std::cout << "}" << std::endl;
	}

	if (fp != NULL) {
		fclose(fp);
	}
	//system("pause");
	return 0;
}