#pragma once
#include "IRCreator.h"
#include <string>
class ScopeExpect {
public:
	ScopeExpect();
	~ScopeExpect();
	// func def
	void meetTypeSpec();
	void canelTypeSpec();
	void meetIdentifier(char *s);
	void meetBracketDeclarator();
	void canelExpect();

	void meetFuncDef();
	void meetIf();
	void meetElse();
	// {
	void meetCurlyBrace();

	void setIRC(IRCreator *irc);
private:
	// func def
	bool hasTypeSpec;
	std::string idName;
	bool isFuncDef;
	bool isIf = false;
	bool isElse = false;
	
	bool isExpecting;
	int numRCBE;//期望的 } 数量

	IRCreator *irc;
};