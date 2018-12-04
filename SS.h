#pragma once
#include <string>
enum class SSType {
	int_const = 0, float_const, char_const, bool_const,
	identifier, func_call, array_left, array_right,
	add, sub, mult, div, mod,
	int_k = 50, float_k, else_k,
	temp_var, ir, label
};

struct IRNode;
struct LabelNode;

struct SSNode {
	SSType type;
	int code_line;
	std::string string_val;
	int int_val;
	float float_val;
	LabelNode *label;

	SSNode(SSNode *n) : string_val() {
		type = n->type;
		string_val = n->string_val;
		int_val = n->int_val;
		float_val = n->float_val;
		code_line = n->code_line;
	}

	SSNode(SSType t, int l, const char *s) : string_val(s) {
		type = t;
		code_line = l;

		if (type == SSType::int_const) {
			int_val = std::stoi(string_val);
		}
		else if (type == SSType::float_const) {
			float_val = std::stof(string_val);
		}
	}

	SSNode(SSType t, int l, int val) : string_val() {
		type = t;
		int_val = val;
		float_val = 0;
		code_line = l;
	}

	SSNode(SSType t, int l, float val) : string_val() {
		type = t;
		float_val = val;
		int_val = 0;
		code_line = l;
	}

	SSNode(LabelNode *l) : string_val() {
		type = SSType::label;
		code_line = -1;
		label = l;
	}


	static bool isConst(SSType t) {
		return t >= SSType::int_const && t <= SSType::bool_const;
	}
};