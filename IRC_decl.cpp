#include "IRCreator.h"


void IRCreator::handle_decl() {
	ss_pop();
}
void IRCreator::handle_idl() {

}
bool IRCreator::handle_init_declarator() {
	int declarator_index = sp_top(), fin_index = ss_len();
	SSNode *id = ss_get(declarator_index), *type_spec = ss_get(declarator_index - 1);
	//std::cout << "HANDLE INIT DECLARATOR " << declarator_index << " " << id->string_val << " " << (int)id->type << std::endl;
	TypeNode *type = stm->getBasicType((int)type_spec->type - 50);

	VarNode *find_res = stm->findCurTable(id->string_val), *insert_res = NULL;
	if (find_res != NULL) {
		// already decl
		return false;
	}
	else {
		insert_res = stm->insert(id->string_val, stm->getCurLevel(), type, id->code_line);
		//std::cout << "HANDLE INIT DECLARATOR " << insert_res->name << std::endl;
	}
	if (declarator_index != fin_index - 1) {
		IRNode *ir = NULL;
		SSNode *initializer = ss_get(declarator_index + 1);
		//std::cout << "HANDLE INIT DECLARATOR " << (int)initializer->type << std::endl;
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
			//ir = new IRNode(IRType::assign, new IRArg(IRAType::float_imm, initializer->float_val), )
			//ir->print();
		}
		addIRNode(ir);
	}

	for (int i = declarator_index; i < fin_index; i++)
		ss_pop();

	return true;
		
}