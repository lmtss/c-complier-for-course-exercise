#include "IRCreator.h"


void IRCreator::handle_decl() {
	ss_pop();
}
void IRCreator::handle_idl() {

}
bool IRCreator::handle_init_declarator() {
	int declarator_index = sp_top(), fin_index = ss_len();
	SSNode *id = ss_get(declarator_index), *type_spec = ss_get(declarator_index - 1);
	TypeNode *type = stm->getBasicType((int)type_spec->type - 50);

	if (id->type == SSType::identifier) {
		VarNode *find_res = stm->findCurTable(id->string_val), *insert_res = NULL;
		if (find_res != NULL) {
			// already decl
			return false;
		}
		else {
			insert_res = stm->insert(id->string_val, stm->getCurLevel(), type, id->code_line);
		}
		if (declarator_index != fin_index - 1) {
			IRNode *ir = NULL;
			SSNode *initializer = ss_get(declarator_index + 1);
			if (initializer->type == SSType::identifier) {
				VarNode *var = NULL;
				_handle_var_undecl(var, initializer);
				ir = new IRNode(IRType::assign, IRAType::var, var, IRAType::var, insert_res);
			}
			else if (initializer->type == SSType::temp_var) {
				ir = new IRNode(IRType::assign, IRAType::temp, initializer->int_val, IRAType::var, insert_res);
			}
			else if (initializer->type == SSType::int_const) {
				ir = new IRNode(IRType::assign, IRAType::int_imm, initializer->int_val, IRAType::var, insert_res);
			}
			else if (initializer->type == SSType::float_const) {

				ir = new IRNode(IRType::assign, IRAType::float_imm, initializer->float_val, IRAType::var, insert_res);
			}
			addIRNode(ir);
		}
	}
	else if (id->type == SSType::func_decl) {
		FuncNode *func = stm->findFunc(id->string_val);
		if (func == NULL) {
			func = new FuncNode;
			func->retType = type;
			func->param_num = 0;
			func->table = NULL;
			for (int i = declarator_index + 1; i < fin_index; i++) {
				VarNode *param = new VarNode;
				SSNode *type_node = ss_get(i);
				//param->declPosLine = type_node->code_line;
				param->varType = stm->getBasicType((int)type_node->type - 50);

				SSNode *id_node = ss_get(i + 1);
				if (id_node->type == SSType::identifier) {
					i++;
				}
				func->paraList[func->param_num++] = param;
				//func->param_num++;
			}
			stm->insertFunc(func);
		}
		else {

		}
	}

	

	for (int i = declarator_index; i < fin_index; i++)
		ss_pop();

	return true;
		
}