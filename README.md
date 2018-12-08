c-complier for course exercise 
=========================== 
编译原理课设，使用递归下降
## 进度
* - [ ] 前端
  * - [x] 函数
  * - [ ] 控制流
    * - [x] if
    * - [ ] while
      * - [x] 基本
      * - [ ] break
      * - [ ] continue
    * - [ ] for
      * - [x] 基本
      * - [ ] break
      * - [ ] continue
    * - [ ] switch
* - [ ] 后端
  * - [ ] 四元式优化-DAG
  * - [ ] 基本块分配
## 支持的语法
参考c-BNF
#### 声明&函数定义
```
translation-unit = decl | func_def

func_def = type-spec id ( {def_param_list}? ) compound-state

decl = type-spec init-declarator-list ;

init-declarator-list = init-declarator {, init-declarator}*

init-declarator = declarator {= initializer}?

declarator = id
           | id ( {decl-param-list}? )

initializer = assign-exp

type-spec = "int" | "float"

decl-param-list = decl-param {, decl-param}*

decl-param = type-spec {id}?

def-param-list = def-param {, def-param}*

decl-param = type-spec id
```
#### 陈述
```
state = compound-state | return-state | if-state | while-state | for-state | exp-state

compound-state = { {{state}|{decl}}* }

exp-state = ;
          | exp ;
if-state = "if" ( logic-or-exp ) state
         | "if" ( logic-or-exp ) state "else" state
while-state = "while" ( logic-or-exp ) state

for-state = "for" ( {assign-exp}? ; logic-or-exp ; {assign-exp}? ) state

return-state = "return" assign-exp ;
```
#### 表达式
```
exp = assign-exp {, assign-exp}*

assign-exp = {assign-left "="}* additive-exp

assign-left = id

additive-exp = multiplicative_exp {{+|-} multiplicative_exp}*

multiplicative_exp = postfix-exp {{*|/} postfix-exp}*

postfix-exp = primary-exp
            | id ( {call-arg-list}? )
primary-exp = id
            | const
            | ( assign-exp )
const = int-const | float-const

logic-or-exp = logic-and-exp {|| logic-and-exp}*

logic-and-exp = rel-exp {|| rel-exp}*

rel-exp = additive-exp {== | > | < | != | >= | <=} additive-exp
```
## 错误信息
```
Undeclared 
Redeclared
ParamNum // 传递参数数量有误
NoRet // 函数定义时，若此函数返回值不是void且没有return语句，报错
```
## 中间代码结构
```cpp
// 总体结构 oper 参数0 参数1 参数2

// 参数
enum class IRAType {
	int_imm, float_imm, 
 var, func,             // 通过 IDNode *id 这个指针获取, 使用时VarNode *v = (VarNode *)id, FuncNode *f = (FuncNode *)id
 temp, NONE, jump_label
};
struct IRArg {
	int int_imm;
	float float_imm;
	int temp_index; // 
	IDNode *id;
	LabelNode *label;
	IRAType type = IRAType::NONE;
}
// 中间代码类型
enum class IRType {
	add, sub, mult, div, assign, 
	ret, // return 语句
	func, func_call, func_param_in, // 函数定义, 函数调用, 函数传参
	equal_jump, unequal_jump, 
	ge_jump, le_jump, // >= <=
	greater_jump, less_jump, 
	jump
};
struct IRNode {
	IRArg args[3]; // 例: 减法 args[2] := args[1] - args[0], 赋值 args[1] := args[0]
	IRType type;
}
```
打印形式
```
{a} := {b} [op] {c} //+-*/
{a} := {b}
if {a} [op] {b} jmp {label}
jmp {label} //无条件跳转
call {func_name} {ret}? //函数调用，若函数返回值为void，则不需要ret
param_in {a} //函数传参
```
## 前端api (大概
```cpp
class FrontEndInterface {
public:
	IRNode *getIR(int i);
	
	std::vector<IRNode *> ir_list;
private:
	
};
```
```cpp
struct VarNode : IDNode {
	TypeNode *varType; // 变量类型, 目前只可能是int
	int level = -1;// 作用域层级 全局作用域为0
};

struct FuncNode : IDNode {
	TypeNode *retType; // 返回值类型, 目前只可能是int
	int param_num;
	vector<VarNode*> paraList;
	SymbolTable *table; // 此函数对应的符号表
};
```
## 寄存器使用 (暂定
$s0 ~ $s6 给分配到寄存器的变量使用

$s7 给未分配到寄存器的变量使用

$t0 ~ $t7 $t8同上

$t9 给特殊寄存器轮换用

