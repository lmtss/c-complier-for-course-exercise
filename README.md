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
   * - [ ] switch
* - [ ] 后端
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

for-state = "for" ( {exp}? ; logic-or-exp ; {exp}? ) state

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
## 中间代码结构
## 符号表api
```cpp

```
