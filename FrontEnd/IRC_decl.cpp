
#include "IRCreator.h"
#include "SymbolTable.h"

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
			RedeclaredError *e = new RedeclaredError(stm->getCurFunc(), id->code_line, id->string_val);
			em->addEN(e);
			return false;
		}
		else if (type == stm->getBasicType(2)) {
			DeclVoidError *e = new DeclVoidError(stm->getCurFunc(), id->code_line, id->string_val);
			em->addEN(e);
			return false;
		}
		else {
			insert_res = stm->insert(id->string_val, stm->getCurLevel(), type, id->code_line);
		}
		if (declarator_index != fin_index - 1) {
			IRNode *ir = NULL;
			SSNode *initializer = ss_get(declarator_index + 1);
			ir = new IRNode(IRType::assign, NULL);
			ir->setArg(1, insert_res);
			_set_arg(ir, 0, initializer);
			if (stm->getCurLevel() == 0)
				addGlobalInit(ir);
			else
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
	else if (id->type == SSType::array) {
		VarNode *find_res = stm->findCurTable(id->string_val), *insert_res = NULL;
		if (find_res != NULL) {
			// already decl
			RedeclaredError *e = new RedeclaredError(stm->getCurFunc(), id->code_line, id->string_val);
			em->addEN(e);
			return false;
		}
		else if (type == stm->getBasicType(2)) {
			DeclVoidError *e = new DeclVoidError(stm->getCurFunc(), id->code_line, id->string_val);
			em->addEN(e);
			return false;
		}
		else {
			SSNode *index = ss_get(declarator_index + 1);
			if (index->type != SSType::int_const) {
				return false;
			}
			
			stm->insert(id->string_val, stm->getCurLevel(), index->int_val, id->code_line);

		}
	}
	

	for (int i = declarator_index; i < fin_index; i++)
		ss_pop();

	return true;
		
}