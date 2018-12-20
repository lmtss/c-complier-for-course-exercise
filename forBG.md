### IRCreator.h 语义分析头文件
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
