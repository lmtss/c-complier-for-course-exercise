#include "IRCreator.h"
#include "AST.h"

IRCreator::IRCreator() {
	head = NULL;
	cur = NULL;

	em = new ErrorManager();
}

IRCreator::~IRCreator() {
}

void IRCreator::setSTM(STManager *s) {
	stm = s;
}



void IRCreator::expState(ASTNode *expStateNode) {
	handleExpression(expStateNode->children[0]);
}

void IRCreator::funcExpect(ASTNode *typeSpecNode, ASTNode *identifier, ASTNode *paramList) {

	FuncNode *func = new FuncNode;

	func->retType = stm->getBasicType(typeSpecNode->children[0]->type - 120);
	func->name = identifier->content;
	stm->addFunc(func);

	FuncIR *ir = new FuncIR;
	ir->next = NULL;
	ir->func = func;
	
	addIRNode(ir);
}

void IRCreator::meetRCB() {
	BlockType type = stm->getCurBlockType();

	if (type == BlockType::func_block) {
		RetIR *ir = new RetIR;
		ir->isTemp = false;
		ir->varIndex = -1;

		addIRNode(ir);
	}

	stm->exitScope();
}

void IRCreator::print() {
	IRNode *printNode = head;
	while (printNode != NULL) {
		printNode->print();
		printNode = printNode->next;
	}
}

// private
void IRCreator::addIRNode(IRNode *node) {
	node->print();
	if (head == NULL) {
		head = node;
		cur = head;
	}
	else {
		cur->next = node;
		cur = node;
	}
}

void IRCreator::handleExpression(ASTNode *exp) {
	/*if (exp->childNum == 1) {
		handleAssign(exp->children[0]);
	}
	else {
		handleExpression(exp->children[0]);
		handleAssign(exp->children[1]);
	}*/
	for (int i = 0; i < exp->childNum; i++) {
		handleAssign(exp->children[i]);
	}
}

TypeNode* IRCreator::handleAssign(ASTNode *assign) {
	if (assign->childNum == 1) {
		return stm->getBasicType(assign->children[0]->children[0]->children[0]->type - 100);
	}
		
	else {
		// check id
		VarNode* var = stm->find(assign->children[0]->content);
		if (var == NULL) {
			// error!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			// id not decl
			UndeclaredError *e = new UndeclaredError(stm->getCurFunc(), assign->codeLine, assign->children[0]->content);
			em->addEN(e);
			return NULL;
		}
		TypeNode* type = handleAssign(assign->children[2]);
		if (type == NULL) {
			return NULL;
		}
		// type conversion

		// create
		if (assign->children[2]->childNum == 1) {
			// var := #10
			AssignImmIR *ir = new AssignImmIR;
			ir->varIndex = var->globalIndex;
			ir->imm = assign->children[2]->children[0]->content;
			ir->isTemp = false;

			addIRNode(ir);
		}
		else {
			// var := var
			AssignIR *ir = new AssignIR;
			ir->varIndex_1 = var->globalIndex;

			VarNode *var2 = stm->find(assign->children[2]->children[0]->content);
			if (var2 == NULL) {

			}
			ir->varIndex_2 = var2->globalIndex;
			ir->isTemp = false;
			//std::cout << assign->children[0]->content << "       " << assign->children[2]->children[0]->content << std::endl;
			addIRNode(ir);
		}
		return var->varType;
	}
}