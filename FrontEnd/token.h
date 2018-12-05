#pragma once

enum Token {
	int_const = 0, float_const, char_const,
	int_k, float_k, char_k, bool_k, void_k,
	return_k, for_k, while_k, if_k, else_k,
	identifier,
	

	//120 { } ( ) . ; , 
	LCB = 20, RCB, LB, RB, LSB, RSB, dot, semicolon, comma,
	// * / + -
	multiply = 29, divide, add, substract,
	logic_or, logic_and,
	// == >= <= > < =
	double_equal = 35, unequal, greater_equal, less_equal, greater, less, assign,
	

	error = 500,
	END = 600
};