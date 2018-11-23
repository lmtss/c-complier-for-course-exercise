#include "AST.h"
#include <cstdarg>
#include <iostream>
ASTNode::ASTNode(ASTType t, int num, ...) : children(4, NULL) {
	va_list args;
	childNum = num;
	type = t;
	va_start(args, num);
	if (num == 0) {
		codeLine = va_arg(args, int);
		content = yytext;
		//std::cout << "AST 0 " << yytext << std::endl;
	}
	else {
		ASTNPtr temp = va_arg(args, ASTNPtr);
		
		codeLine = temp->codeLine;
		//std::cout << "0";
		if (num == 1) {
			if (temp->content.size() > 0) {
				content = temp->content;
			}
			else
				content = "";
		}
		//std::cout << "1";
		children[0] = temp;
		for (int i = 1; i < num; i++) {
			temp = va_arg(args, ASTNPtr);
			children[i] = temp;
		}
	}
}
ASTNode::~ASTNode() {
	for (int i = 0; i < childNum; i++) {
		delete children[i];
	}
}
void ASTNode::print(){
	std::cout << "line:" << codeLine << "  " << childNum << std::endl;
}
void ASTNode::addChild(ASTNode *node) {
	children[childNum++] = node;
}