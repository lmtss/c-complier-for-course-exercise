#pragma once
#include "IRCreator.h"
#include <string>
class ScopeExpect {
public:
	ScopeExpect();
	~ScopeExpect();
	// func def
	void meetTypeSpec(ASTNode *node);
	void canelTypeSpec();
	void meetIdentifier(char *s);
	void meetBracketDeclarator(ASTNode *node);
	void canelExpect();

	void meetFuncDef(ASTNode *type, ASTNode *identifier, ASTNode *param_list);
	// {
	void meetCurlyBrace();

	void setIRC(IRCreator *irc);
private:
	// func def
	bool hasTypeSpec;
	std::string idName;
	ASTNode *typeSpecNode, *bracketDeclarator, *paramList, *id;
	bool isFuncDef;
	
	bool isExpecting;
	int numRCBE;//期望的 } 数量

	IRCreator *irc;
};