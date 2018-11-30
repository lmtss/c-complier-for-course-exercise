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

void ScopeExpect::meetTypeSpec() {
	hasTypeSpec = true;
}

void ScopeExpect::canelTypeSpec() {
	hasTypeSpec = false;
}

void ScopeExpect::meetIdentifier(char *s) {
	if (hasTypeSpec)
		idName = s;
}

void ScopeExpect::meetBracketDeclarator() {
	if (hasTypeSpec)
		isExpecting = true;

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
		irc->funcExpect();
	}
	

}

void ScopeExpect::setIRC(IRCreator *i) {
	irc = i;
}

void ScopeExpect::meetFuncDef() {
	isFuncDef = true;
}