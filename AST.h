#pragma once
#include<string>
#include<iostream>
#include<vector>

extern char *yytext;

enum ASTType {
	//yacc
	AST_const = 0, AST_type_spec, AST_assignment_oper, AST_assignment_exp, AST_exp, AST_initializer,
	AST_declarator, AST_init_declarator, AST_init_declarator_list, AST_decl, AST_decl_list,
	AST_statement, AST_exp_statement, AST_compound_statement, AST_block_item_list, AST_block_item,
	AST_func_def, AST_translation_unit, AST_external_decl, AST_Program,
	//yacc ¹ý³Ì
	AST_func_declarator = 50,// declarator:declarator()
	AST_var,// declarator:identifier
	//exp
	AST_primary_exp, AST_assign_left, AST_postfix_exp,
	//lex
	AST_identifier = 90, 
	AST_int_const = 100, AST_float_const, AST_char_const, AST_string_const, AST_enum_const,
	AST_int_k = 120, AST_float_k, AST_char_k, AST_void_k,
	//·ûºÅ
	AST_mult, AST_div, AST_mod, AST_add, AST_sub
};

enum ASTSubType {
	func_del, var_decl
};

struct ASTNode {
	std::string content;
	ASTType type;
	int childNum;
	int codeLine;
	std::vector<ASTNode *> children;
	//ASTNode *children[4];

	ASTNode(ASTType t, int num, ...);
	void addChild(ASTNode *node);
	~ASTNode();
	void print();
};
typedef ASTNode* ASTNPtr;