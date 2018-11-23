#include "IRCreator.h"

void IRCreator::decl(ASTNode *declNode) {
	handleIDL(
		stm->getBasicType(declNode->children[0]->children[0]->type - 120),
		declNode->children[1]
	);
}

// private
void IRCreator::handleIDL(TypeNode *type, ASTNode *idl) {
	/*if (idl->childNum == 1) {
		handleInitDeclarator(type, idl->children[0]);
	}
	else {
		handleIDL(type, idl->children[0]);
		handleInitDeclarator(type, idl->children[1]);
	}*/
	for (int i = 0; i < idl->childNum; i++) {
		handleInitDeclarator(type, idl->children[i]);
	}
}

void IRCreator::handleInitDeclarator(TypeNode *type, ASTNode *node) {
	// decl func
	if (node->children[0]->type == ASTType::AST_func_declarator) {
		// check whole scope
		if (stm->getCurLevel() != 0) {
			// error!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			// func not decl in wholeblock

		}

		// check 




	}
	else {// decl var
		// check id
		VarNode *var = stm->findCurTable(node->children[0]->content);
		//std::cout << node->children[0]->content << " " << (int)node->type << std::endl;
		if (var != NULL) {
			// error!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			// var already decl
			
			RedeclaredError *e = new RedeclaredError(stm->getCurFunc(), node->codeLine, node->children[0]->content);
			em->addEN(e);
			return;

		}
		else {// insert
			var = stm->insert(node->children[0]->content, stm->getCurLevel(), type, node->codeLine);

		}

		// init
		if (node->childNum == 2) {
			// handle initializer
			ASTNode *assignExp = node->children[1]->children[0];
			TypeNode *rightType = handleAssign(assignExp);
			// type conversion
			// 
			if (assignExp->childNum == 1) {
				// var := #10
				AssignImmIR *ir = new AssignImmIR;
				ir->varIndex = var->globalIndex;
				ir->imm = assignExp->children[0]->content;
				ir->isTemp = false;

				addIRNode(ir);
			}
			else {
				// var := var
				AssignIR *ir = new AssignIR;
				ir->varIndex_1 = var->globalIndex;

				VarNode *var2 = stm->find(assignExp->children[0]->content);
				if (var2 == NULL) {

				}
				ir->varIndex_2 = var2->globalIndex;
				ir->isTemp = false;

				addIRNode(ir);
			}
		}
	}
}