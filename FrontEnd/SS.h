#pragma once
#include <string>
enum class SSType {
	int_const = 0, float_const, char_const, bool_const,
	identifier, func_call, func_decl, array_left, array_right,
	add, sub, mult, div, mod,
	int_k = 50, float_k, void_k, else_k, 
	temp_var, ir, label,array
};

struct IRNode;
struct LabelNode;
struct TempNode;

struct SSNode {
	SSType type;
	int code_line;
	std::string string_val;
	int int_val;
	float float_val;
	char char_val;
	LabelNode *label;
	TempNode *temp;

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
		else if (type == SSType::char_const) {
			if (string_val.length() > 3) {
				if (string_val[2] == 'n')
					char_val = '\n';
			}
			else if (string_val.length() == 2) {
				char_val = ' ';
			}
			else {
				char_val = string_val[1];
			}
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

	SSNode(TempNode *t) : string_val() {
		temp = t;
		type = SSType::temp_var;
		code_line = -1;
	}


	static bool isConst(SSType t) {
		return t >= SSType::int_const && t <= SSType::bool_const;
	}
};