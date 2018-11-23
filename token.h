#pragma once

enum Token {
	int_const = 0, float_const, char_const,
	int_k, float_k, char_k, bool_k, void_k,
	return_k, for_k, while_k, if_k,
	identifier,

	//120 { } ( ) . ; , 
	LCB = 20, RCB, LB, RB, LSB, RSB, dot, semicolon, comma,
	// * / + -
	multiply = 29, divide, add, substract,
	// == >= <= > < =
	double_equal = 33, greater_equal, less_equal, greater, less, assign,

	error = 500,
	END = 600
};