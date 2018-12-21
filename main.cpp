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

int main(int argc, char *argv[]) {// 参数格式为 源文件地址 汇编文件地址 true/false
	
	yytext = new char[100];
	yytext[0] = '\0';

	FILE *fp = NULL;// 源文件
	FILE *asm_save_fp = NULL;
	fstream out;// 输出流
	bool is_print_to_json = false;// 是否以json格式输出

	// 解析argc argv 获取参数
	if (argc > 1) {
		fp = fopen(argv[1], "r");// 从参数获取源文件地址
		if (argc > 2) {
			out.open(argv[2], ios::out);// 获取输出流
			if (argc > 3 && argv[3][0] == 't') {
				is_print_to_json = true;// 是否输出为json
			}
		}
		else {
			out.open("C:/Users/Lenovo-/Desktop/asm.s", ios::out);// 默认汇编文件地址
		}
	}
	else {
		fp = fopen("C:/Users/Lenovo-/Desktop/tt.c", "r");// 默认源文件地址
		out.open("C:/Users/Lenovo-/Desktop/asm.s", ios::out);
	}

	lex = new Lex();// 词法分析器
	lex->setFP(fp);
	/*while (true) {
		Token t = lex->lex();
		if (t >= Token::error)
			break;
	}*/
	//lex->print();

	scopePredictor = new ScopeExpect();
	irCreator = new IRCreator();// 语义分析器
	stManager = new STManager();// 符号表
	irCreator->setSTM(stManager);
	scopePredictor->setIRC(irCreator);
	
	Parser *parser = new Parser();// 语法分析器
	parser->lex = [] {return lex->lex(); };// 使语法分析器能够调用词法分析器
	parser->set(scopePredictor, irCreator);

	parser->parse();// 开始语法分析
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
	DAG dag(FEI); // 中间代码优化
	dag.gene_blocks();
	dag.optimize();
	//FEI->adjust();
	if (is_print_to_json)
		FEI->print_for_json();
	else
		FEI->print();
	RegAllocator *alloc = new RegAllocator(FEI);// 寄存器分配
	ASMCreator *ac = new ASMCreator(FEI, alloc, out, is_print_to_json);// 汇编生成
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