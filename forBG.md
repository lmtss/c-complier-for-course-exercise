## IRCreator.h 语义分析头文件
```cpp
class IRCreator {
public:
	void enter_scope();// 进入一个作用域

	void print();// 以普通控制台格式打印
	void print_json();// 以json格式输出

	void new_sp();// 创建一个锚点，用于错误回复等。

	void pop_sp(); // 取消一个锚点
	void ss_wrong_re(); // 语义栈的错误恢复


	void ss_push(SSNode *n);// 语义栈push

	SSNode *ss_top() { return ss_stack[ss_index - 1]; }// 语义栈top
	void ss_pop(); // 语义栈pop
	
	void handle_token_error(int line, Token t);// 处理读到错误token类型的错误

	// 语义动作
	bool handle_assign_exp();
	bool handle_additive_exp();
	bool handle_multiplicative_exp();
	void handle_decl();
	void handle_idl();
	bool handle_init_declarator();
	bool handle_func_def();
	bool handle_func_call();
	bool handle_return_state();
	bool handle_print_state();
	bool handle_in_state();
	bool handle_in_arg();
	bool handle_state();
	bool handle_logic_exp();
	bool handle_logic_or_exp_1();
	bool handle_logic_or_exp_2();
	bool handle_logic_and_exp_1();
	bool handle_logic_and_exp_2();
	bool handle_rel_exp(Token op);
	bool handle_if_state_1();
	bool handle_if_state_2();
	bool handle_if_state_3();
	bool handle_if_state_4();
	bool handle_while_state();
	bool handle_while_state_1();
	bool handle_while_state_2();
	bool handle_for_state_1();
	bool handle_for_state_2();
	bool handle_for_state_3();
	bool handle_array_use();
  
  	// 用于回填
	bool is_parse_if = false;
	bool is_parse_else = false;
	bool is_parse_if_end = false;
	IRNode *rel_exp_first = NULL, *and_exp_first = NULL;
	bool expect_for_rel_exp = false, expect_for_and_exp = false;
	IRNode *for_exp_3_head = NULL, *for_exp_3_end = NULL;
	bool expect_for_exp_3 = false;
	LabelNode *_expect_true_label = NULL, *_expect_false_label = NULL, *_expect_end_label = NULL;
	LabelNode *_expect_while_to_logic_label = NULL;

	void adjust();

private:
	int temp_top_index = 0;// 当前temp

	std::vector<SSNode*> ss_stack;// 语义栈
	std::vector<int> ss_sp_stack;

	std::map<IRNode*, LabelNode*> label_map;// 标签map

	void addIRNode(IRNode *node);// 处理新生成中间代码。处理回填。
	void addGlobalInit(IRNode *ir);// 将全局变量相关的代码转移位置，方便后端处理。

	bool set_arg(IRNode *ir, int index, SSNode *n);// 一个方便的设定中间代码参数的函数


	void label_finish(LabelNode *label);// 完成一个跳转标签，即回填。保证同一个代码不会有两个跳转标签。

};

```
## ASMCreator.h 汇编代码生成头文件
```cpp
class ASMCreator {
public:
	void create_head();// 创建汇编头部(数据段等等)
	void create_block(int start, int end);// 按照区间生成汇编
private:
	// 生成函数，对应每一种中间代码
	void create_func(IRNode *ir); 
	void create_assign(IRNode *ir);
	void create_asmd(IRNode *ir);
	void create_print(IRNode *ir);
	void create_input(IRNode *ir, IRNode *next);
	void create_jump(IRNode *ir);
	void create_array_assign(IRNode *ir);
	void create_array_use(IRNode *ir);
	void create_param_in(IRNode *ir);
	void create_call(IRNode *ir);
	void create_call_fin();
	void create_return(IRNode *ir);

	int load_store(IRNode *ir, int i, bool use_t9);// 处理变量读取/存储
	void store(VarNode *var);// 变量存储
	void handle_t9(int imm);// 将立即数imm存入$t9
	void handle_t9(VarNode *var, int add);// 将变量var存入$t9 或 将数组元素的地址存入$t9
	int var_offset(VarNode *var);// 获取变量偏移量
	int var_offset(TempNode *temp);// 获取临时变量偏移量

	ASMOut out;// 输出流
	
};
```
## main.cpp
```cpp
int main(int argc, char *argv[]) {// 参数格式为 源文件地址 汇编文件地址 true/false
	
	yytext = new char[100];
	yytext[0] = '\0';

	FILE *fp = NULL;// 源文件
	fstream out;// 输出流
	bool is_print_to_json = false;// 是否以json格式输出

	// 解析argc argv 获取参数
	if (argc > 1) {
		fp = fopen(argv[1], "r");// 从参数获取源文件地址
		if (argc > 2) {
			out.open(argv[2], ios::out);// 获取输出流
			if (argc > 3 && argv[3][0] == 't') {
				is_print_to_json = true;// 是否输出为json
			}
		}
		else {
			out.open("C:/Users/Lenovo-/Desktop/asm.s", ios::out);// 默认汇编文件地址
		}
	}
	else {
		fp = fopen("C:/Users/Lenovo-/Desktop/tt.c", "r");// 默认源文件地址
		out.open("C:/Users/Lenovo-/Desktop/asm.s", ios::out);
	}

	lex = new Lex();// 词法分析器
	lex->setFP(fp);

	scopePredictor = new ScopeExpect();
	irCreator = new IRCreator();// 语义分析器
	stManager = new STManager();// 符号表
	irCreator->setSTM(stManager);
	scopePredictor->setIRC(irCreator);
	
	Parser *parser = new Parser();// 语法分析器
	parser->lex = [] {return lex->lex(); };// 使语法分析器能够调用词法分析器
	parser->set(scopePredictor, irCreator);

	parser->parse();// 开始语法分析
	irCreator->adjust();

	if (is_print_to_json) {
		std::cout << "{" << std::endl;
		lex->print_for_json();
		irCreator->print_json();
	}
	else {
		irCreator->print();
	}

	FrontEndInterface *FEI = new FrontEndInterface(irCreator, stManager);
	
	DAG dag(FEI); // 中间代码优化
	dag.gene_blocks();
	dag.optimize();

	if (is_print_to_json)
		FEI->print_for_json();
	else
		FEI->print();
		
	RegAllocator *alloc = new RegAllocator(FEI);// 寄存器分配
	ASMCreator *ac = new ASMCreator(FEI, alloc, out, is_print_to_json);// 汇编生成
	ac->create_head();

	if (is_print_to_json) {
		std::cout << std::endl;
		std::cout << "}" << std::endl;
	}

	if (fp != NULL) {
		fclose(fp);
	}

	return 0;
}
```
```cpp
int a = 1;
int arr[10];
int gcd(int u, intv){...}
int main(){
  int c = 4;
  for(a = 0; a < 10; a = a + 1){
    int b = 2;
  }
}
```
```cpp
// 符号表类
class SymbolTable {
public:

	VarNode* find(string& str) const;// 插入变量
	void insert(string& name, VarNode *id);// 插入变量
	void traverse(function<void(VarNode*)> func);// 遍历自己的符号表
	void back_traverse(function<void(VarNode*)> func); // 回溯遍历， 遍历当前符号表并沿父指针向上遍历符号表

	SymbolTable *father;// 指向父符号表的指针
	BlockType type;// 符号表类型， 可能是函数、for等等
	map<string, VarNode*> table;// 变量表
};

//符号表管理类
class STManager {
public:
	void addTable(BlockType t);// 添加一个符号表并进入此符号表(进入新的作用域)
	SymbolTable* addFunc(FuncNode *func);// 添加一个函数(进入新的作用域)
	void exitScope();// 离开当前作用域

	VarNode* find(string& str) const;// 回溯查找
	VarNode* findCurTable(string& str) const;// 仅在当前作用域查找
	FuncNode* findFunc(string& str) const;// 查找函数
	VarNode* insert(string& name, int level, TypeNode *type, int line);// 插入变量
	void insert(string& name, int level, int len, int line);// 插入变量

	void insert(int i);// 插入临时变量
	void insert(TempNode *temp);// 插入临时变量
	void insertFunc(FuncNode *f);

	void func_traverse(function<void(VarNode*)> func);// 遍历所有函数节点

	TypeNode *getBasicType(int i);// 获取基本变量类型

	int getCurLevel() {return curLevel;}// 获取当前作用域层级
	BlockType getCurBlockType() { return curTable->type; }
	FuncNode *getCurFunc() { return curFunc; }// 获取当前函数
	SymbolTable *getWholeTable() { return wholeTable; }
	SymbolTable *getCurTable() { return curTable; }// 获取当前符号表

	vector<TempNode*> temp_list;
private:

	int cur_type_index = 4;
	map<string, FuncNode*> funcTable;

};
```
```cpp
//中间代码操作数
struct IRArg {
	int int_imm;
	float float_imm;
	int temp_index;
	char char_imm;
	TempNode *temp;
	IDNode *id;
	LabelNode *label;

	IRAType type = IRAType::NONE;	
};
//中间代码
struct IRNode {
	IRArg args[3];//三个操作数
	IRType type;
	IRNode *next, *front;
	LabelNode *label = NULL;//跳转标签
	
	SymbolTable *scope = NULL;//产生时所在的作用域
	FuncNode *func = NULL;// 产生时位于的函数
};
```
