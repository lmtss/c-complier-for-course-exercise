#include "IRCreator.h"
#include "ScopeExpect.h"
#include<iostream>
ScopeExpect::ScopeExpect() {
	hasTypeSpec = false;
	isExpecting = false;
	isFuncDef = false;
}

ScopeExpect::~ScopeExpect() {
}

void ScopeExpect::meetTypeSpec(ASTNode *node) {
	hasTypeSpec = true;
	typeSpecNode = node;
}

void ScopeExpect::canelTypeSpec() {
	hasTypeSpec = false;
}

void ScopeExpect::meetIdentifier(char *s) {
	if (hasTypeSpec)
		idName = s;
}

void ScopeExpect::meetBracketDeclarator(ASTNode *node) {
	if (hasTypeSpec)
		isExpecting = true;
	//hasTypeSpec = false;

	bracketDeclarator = node;
}

void ScopeExpect::canelExpect() {
	isExpecting = false;
}

void ScopeExpect::meetCurlyBrace() {
	isExpecting = false;

	// func
	if (isFuncDef) {
		hasTypeSpec = false;
		isFuncDef = false;
		irc->funcExpect(typeSpecNode,id,paramList);
	}
	

}

void ScopeExpect::setIRC(IRCreator *i) {
	irc = i;
}

void ScopeExpect::meetFuncDef(ASTNode *type, ASTNode *identifier, ASTNode *param_list) {
	typeSpecNode = type;
	paramList = param_list;
	id = identifier;
	isFuncDef = true;
}