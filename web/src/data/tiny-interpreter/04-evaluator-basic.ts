import type { LearningModule } from "../types";

export const module04EvaluatorBasic: LearningModule = {
  id: "04-evaluator-basic",
  index: 4,
  slug: "evaluator-basic",
  hasCode: true,
  vizType: "evaluator",
  diagnostics: [
    { pattern: "_allFail", message: { zh: "从 `eval()` 的分支开始——Number 和 Boolean 直接返回 `.value`", en: "Start with `eval()` branches — Number and Boolean just return `.value`" } },
    { pattern: "test_if*", message: { zh: "`if` 是特殊形式——只求值一个分支，不是两个都求值", en: "`if` is a special form — only evaluate one branch, not both" } },
    { pattern: "test_quote*", message: { zh: "`quote` 不求值参数——直接把 AST 转成值返回", en: "`quote` doesn't evaluate its argument — return the AST as a value directly" } },
    { pattern: "test_define*", message: { zh: "`eval_define()` 需要先求值右边的表达式，再存到环境里", en: "`eval_define()` needs to evaluate the right-hand expression first, then store it in the environment" } },
    { pattern: ["test_arithmetic", "test_nested_arithmetic"], message: { zh: "函数调用：先求值所有参数，再调用函数", en: "Function calls: evaluate all arguments first, then call the function" } },
  ],
  skeleton: `"""基础求值器骨架代码。

你的任务是实现标记为 TODO 的方法。

运行测试：
    pytest learn/04-evaluator-basic/test_skeleton.py -v

注意：
    这个模块只实现基础求值,不包括 lambda。
    lambda 和闭包在模块 5 中实现。
"""

from typing import Any, List

import sys
import os
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..'))

from src.tiny_interpreter.parser import ASTNode, Number, Boolean, Symbol, SExpression, parse
from src.tiny_interpreter.environment import Environment


class EvaluatorError(Exception):
    """求值错误。"""
    pass


class Evaluator:
    """求值器：执行 AST 节点。

    使用方法：
        evaluator = Evaluator()
        result = evaluator.run("(+ 1 2)")
    """

    def __init__(self):
        """初始化求值器,创建全局环境。"""
        self.global_env = self.create_global_environment()

    def create_global_environment(self) -> Environment:
        """创建包含内置函数的全局环境。

        内置函数包括：
        - 算术：+, -, *, /
        - 比较：=, <, >, <=, >=
        - 列表：cons, car, cdr, list, null?
        - 类型：number?, boolean?, list?
        """
        env = Environment()

        # 算术运算
        env.define('+', lambda *args: sum(args))
        env.define('-', lambda a, b: a - b)
        env.define('*', lambda *args: 1 if not args else args[0] if len(args) == 1 else args[0] * self.global_env.get('*')(*args[1:]))
        env.define('/', lambda a, b: a // b)

        # 比较运算
        env.define('=', lambda a, b: a == b)
        env.define('<', lambda a, b: a < b)
        env.define('>', lambda a, b: a > b)
        env.define('<=', lambda a, b: a <= b)
        env.define('>=', lambda a, b: a >= b)

        # 列表操作
        env.define('cons', lambda a, b: [a] + (b if isinstance(b, list) else [b]))
        env.define('car', lambda lst: lst[0] if lst else None)
        env.define('cdr', lambda lst: lst[1:] if len(lst) > 1 else [])
        env.define('list', lambda *args: list(args))
        env.define('null?', lambda lst: len(lst) == 0 if isinstance(lst, list) else False)

        # 类型判断
        env.define('number?', lambda x: isinstance(x, int))
        env.define('boolean?', lambda x: isinstance(x, bool))
        env.define('list?', lambda x: isinstance(x, list))

        return env

    def eval(self, node: ASTNode, env: Environment) -> Any:
        """求值一个 AST 节点。

        TODO: 实现这个方法

        Args:
            node: AST 节点
            env: 当前环境

        Returns:
            求值结果

        求值规则：
        1. Number → 返回 node.value
        2. Boolean → 返回 node.value
        3. Symbol → 在环境中查找 env.get(node.name)
        4. SExpression → 特殊形式或函数调用

        对于 SExpression：
        - 如果是空列表 () → 返回空列表 []
        - 如果第一个元素是 Symbol：
          - "define" → 调用 eval_define
          - "if" → 调用 eval_if
          - "quote" → 调用 eval_quote
          - "begin" → 调用 eval_begin
          - 否则 → 函数调用,调用 eval_application
        - 如果第一个元素不是 Symbol → 函数调用
        """
        # TODO: 实现
        pass

    def eval_define(self, args: List[ASTNode], env: Environment) -> None:
        """求值 define 表达式。

        TODO: 实现这个方法

        形式：(define name value)

        Args:
            args: define 的参数列表 [name, value]
            env: 当前环境

        Returns:
            None

        步骤：
        1. 检查参数数量是否为 2
        2. 检查第一个参数是否是 Symbol
        3. 求值第二个参数
        4. 在环境中定义变量
        """
        # TODO: 实现
        pass

    def eval_if(self, args: List[ASTNode], env: Environment) -> Any:
        """求值 if 表达式。

        TODO: 实现这个方法

        形式：(if condition then-expr else-expr)

        Args:
            args: if 的参数列表 [condition, then-expr, else-expr]
            env: 当前环境

        Returns:
            then-expr 或 else-expr 的值

        步骤：
        1. 检查参数数量是否为 3
        2. 求值条件
        3. 如果条件为真,求值并返回 then-expr
        4. 否则求值并返回 else-expr

        注意：只求值需要的分支！
        """
        # TODO: 实现
        pass

    def eval_quote(self, args: List[ASTNode]) -> Any:
        """求值 quote 表达式。

        TODO: 实现这个方法

        形式：(quote expr)

        Args:
            args: quote 的参数列表 [expr]

        Returns:
            expr 转换为值（不求值）

        步骤：
        1. 检查参数数量是否为 1
        2. 调用 ast_to_value 转换 AST 为值
        """
        # TODO: 实现
        pass

    def eval_begin(self, args: List[ASTNode], env: Environment) -> Any:
        """求值 begin 表达式。

        TODO: 实现这个方法

        形式：(begin expr1 expr2 ... exprN)

        Args:
            args: begin 的参数列表 [expr1, expr2, ...]
            env: 当前环境

        Returns:
            最后一个表达式的值

        步骤：
        1. 依次求值每个表达式
        2. 返回最后一个表达式的值
        """
        # TODO: 实现
        pass

    def eval_application(self, elements: List[ASTNode], env: Environment) -> Any:
        """求值函数调用。

        TODO: 实现这个方法

        形式：(func arg1 arg2 ...)

        Args:
            elements: S-表达式的所有元素 [func, arg1, arg2, ...]
            env: 当前环境

        Returns:
            函数调用的结果

        步骤：
        1. 求值第一个元素,得到函数
        2. 求值其余元素,得到参数列表
        3. 调用函数

        注意：这里只处理内置函数（callable）。
        用户定义的函数（Closure）在模块 5 中处理。
        """
        # TODO: 实现
        pass

    def ast_to_value(self, node: ASTNode) -> Any:
        """将 AST 节点转换为值（用于 quote）。

        Args:
            node: AST 节点

        Returns:
            对应的值
        """
        if isinstance(node, Number):
            return node.value
        if isinstance(node, Boolean):
            return node.value
        if isinstance(node, Symbol):
            return node.name  # 符号变成字符串
        if isinstance(node, SExpression):
            return [self.ast_to_value(elem) for elem in node.elements]
        return node

    def run(self, source: str) -> Any:
        """解析并执行源代码。

        Args:
            source: 源代码字符串

        Returns:
            最后一个表达式的值
        """
        ast_nodes = parse(source)
        result = None
        for node in ast_nodes:
            result = self.eval(node, self.global_env)
        return result`,
  testCode: `"""基础求值器测试。

运行测试：
    pytest learn/04-evaluator-basic/test_skeleton.py -v
"""

import pytest
from skeleton import Evaluator, EvaluatorError


@pytest.fixture
def evaluator():
    """创建求值器实例。"""
    return Evaluator()


class TestSelfEvaluating:
    """自求值表达式测试。"""

    def test_number(self, evaluator):
        """测试数字求值。"""
        assert evaluator.run("42") == 42
        assert evaluator.run("-10") == -10

    def test_boolean(self, evaluator):
        """测试布尔值求值。"""
        assert evaluator.run("#t") is True
        assert evaluator.run("#f") is False


class TestArithmetic:
    """算术运算测试。"""

    def test_addition(self, evaluator):
        """测试加法。"""
        assert evaluator.run("(+ 1 2)") == 3
        assert evaluator.run("(+ 1 2 3)") == 6

    def test_subtraction(self, evaluator):
        """测试减法。"""
        assert evaluator.run("(- 5 3)") == 2

    def test_multiplication(self, evaluator):
        """测试乘法。"""
        assert evaluator.run("(* 3 4)") == 12

    def test_division(self, evaluator):
        """测试除法（整数除法）。"""
        assert evaluator.run("(/ 10 3)") == 3

    def test_nested_arithmetic(self, evaluator):
        """测试嵌套算术。"""
        assert evaluator.run("(+ 1 (* 2 3))") == 7
        assert evaluator.run("(* (+ 1 2) (- 5 3))") == 6


class TestComparison:
    """比较运算测试。"""

    def test_equal(self, evaluator):
        """测试相等。"""
        assert evaluator.run("(= 1 1)") is True
        assert evaluator.run("(= 1 2)") is False

    def test_less_than(self, evaluator):
        """测试小于。"""
        assert evaluator.run("(< 1 2)") is True
        assert evaluator.run("(< 2 1)") is False

    def test_greater_than(self, evaluator):
        """测试大于。"""
        assert evaluator.run("(> 2 1)") is True
        assert evaluator.run("(> 1 2)") is False


class TestDefine:
    """define 测试。"""

    def test_define_number(self, evaluator):
        """测试定义数字变量。"""
        evaluator.run("(define x 42)")
        assert evaluator.run("x") == 42

    def test_define_expression(self, evaluator):
        """测试定义表达式结果。"""
        evaluator.run("(define x (+ 1 2))")
        assert evaluator.run("x") == 3

    def test_use_defined_variable(self, evaluator):
        """测试使用定义的变量。"""
        evaluator.run("(define x 10)")
        evaluator.run("(define y 20)")
        assert evaluator.run("(+ x y)") == 30


class TestIf:
    """if 测试。"""

    def test_if_true(self, evaluator):
        """测试条件为真。"""
        assert evaluator.run("(if #t 1 2)") == 1

    def test_if_false(self, evaluator):
        """测试条件为假。"""
        assert evaluator.run("(if #f 1 2)") == 2

    def test_if_with_comparison(self, evaluator):
        """测试带比较的条件。"""
        assert evaluator.run("(if (< 1 2) 10 20)") == 10
        assert evaluator.run("(if (> 1 2) 10 20)") == 20

    def test_if_nested(self, evaluator):
        """测试嵌套 if。"""
        code = "(if #t (if #f 1 2) 3)"
        assert evaluator.run(code) == 2


class TestQuote:
    """quote 测试。"""

    def test_quote_number(self, evaluator):
        """测试引用数字。"""
        assert evaluator.run("(quote 42)") == 42

    def test_quote_symbol(self, evaluator):
        """测试引用符号。"""
        assert evaluator.run("(quote x)") == "x"

    def test_quote_list(self, evaluator):
        """测试引用列表。"""
        assert evaluator.run("(quote (1 2 3))") == [1, 2, 3]

    def test_quote_nested(self, evaluator):
        """测试引用嵌套列表。"""
        assert evaluator.run("(quote (+ 1 2))") == ["+", 1, 2]


class TestBegin:
    """begin 测试。"""

    def test_begin_single(self, evaluator):
        """测试单个表达式。"""
        assert evaluator.run("(begin 42)") == 42

    def test_begin_multiple(self, evaluator):
        """测试多个表达式。"""
        assert evaluator.run("(begin 1 2 3)") == 3

    def test_begin_with_define(self, evaluator):
        """测试带 define 的 begin。"""
        result = evaluator.run("""
            (begin
                (define x 10)
                (define y 20)
                (+ x y))
        """)
        assert result == 30


class TestListOperations:
    """列表操作测试。"""

    def test_list(self, evaluator):
        """测试创建列表。"""
        assert evaluator.run("(list 1 2 3)") == [1, 2, 3]

    def test_car(self, evaluator):
        """测试 car。"""
        assert evaluator.run("(car (list 1 2 3))") == 1

    def test_cdr(self, evaluator):
        """测试 cdr。"""
        assert evaluator.run("(cdr (list 1 2 3))") == [2, 3]

    def test_cons(self, evaluator):
        """测试 cons。"""
        assert evaluator.run("(cons 1 (list 2 3))") == [1, 2, 3]

    def test_null(self, evaluator):
        """测试 null?。"""
        assert evaluator.run("(null? (list))") is True
        assert evaluator.run("(null? (list 1))") is False


class TestTypePredicates:
    """类型判断测试。"""

    def test_number_predicate(self, evaluator):
        """测试 number?。"""
        assert evaluator.run("(number? 42)") is True
        assert evaluator.run("(number? #t)") is False

    def test_boolean_predicate(self, evaluator):
        """测试 boolean?。"""
        assert evaluator.run("(boolean? #t)") is True
        assert evaluator.run("(boolean? 42)") is False

    def test_list_predicate(self, evaluator):
        """测试 list?。"""
        assert evaluator.run("(list? (list 1 2))") is True
        assert evaluator.run("(list? 42)") is False


class TestEmptyList:
    """空列表测试。"""

    def test_empty_sexp(self, evaluator):
        """测试空 S-表达式。"""
        assert evaluator.run("()") == []`,
  readme: {
    zh: `# 模块 4：基础求值器 (Evaluator - Basic)

> "表达式的值是怎么算出来的？"

## 问题引入

我们已经有了：
- **词法分析器**：把源代码变成 token
- **解析器**：把 token 变成 AST
- **环境**：存储变量绑定

现在的问题是：**如何计算一个表达式的值？**

\`\`\`lisp
(+ 1 (* 2 3))  ; 结果应该是 7
\`\`\`

这看起来简单，但仔细想想：
- \`(* 2 3)\` 需要先算出 6
- 然后 \`(+ 1 6)\` 才能算出 7
- 求值是**递归**的！

---

## 核心概念

### 不同节点类型的求值规则

| 节点类型 | 求值规则 | 例子 |
|---------|---------|------|
| Number | 直接返回值 | \`42\` → \`42\` |
| Boolean | 直接返回值 | \`#t\` → \`True\` |
| Symbol | 在环境中查找 | \`x\` → \`env.get("x")\` |
| SExpression | 特殊形式或函数调用 | \`(+ 1 2)\` → \`3\` |

### 自求值表达式

数字和布尔值是"自求值"的——它们的值就是它们自己：

\`\`\`python
eval(Number(42))  → 42
eval(Boolean(True))  → True
\`\`\`

### 变量查找

符号（Symbol）需要在环境中查找：

\`\`\`python
eval(Symbol("x"))  → env.get("x")
\`\`\`

### 函数调用

S-表达式的求值最复杂：

\`\`\`python
eval((+ 1 (* 2 3)))
# 1. 求值 +  → 得到加法函数
# 2. 求值 1  → 得到 1
# 3. 求值 (* 2 3)  → 递归求值，得到 6
# 4. 调用加法函数(1, 6)  → 7
\`\`\`

---

## 特殊形式

有些 S-表达式不是函数调用，而是**特殊形式**——它们有自己的求值规则。

### define

\`\`\`lisp
(define x 42)
\`\`\`

不是调用 \`define\` 函数，而是在环境中创建绑定。

### if

\`\`\`lisp
(if (< 1 2) 10 20)  ; → 10
\`\`\`

**为什么 if 是特殊形式？** 因为它不能求值所有参数！

如果 \`if\` 是普通函数，那么 \`then\` 和 \`else\` 分支都会被求值。但 \`if\` 只应该求值其中一个分支。

### quote

\`\`\`lisp
(quote (+ 1 2))  ; → ["+", 1, 2]（不求值，返回数据）
\`\`\`

\`quote\` 阻止求值，把代码当作数据返回。

### begin

\`\`\`lisp
(begin
  (define x 10)
  (define y 20)
  (+ x y))  ; → 30
\`\`\`

\`begin\` 依次求值每个表达式，返回最后一个的值。

---

## 动手实现

### 步骤 1：实现 eval() 的基本分发

先处理 Number、Boolean、Symbol 三种简单情况。

### 步骤 2：处理 SExpression

区分特殊形式（define、if、quote、begin）和普通函数调用。

### 步骤 3：实现各个特殊形式

- \`eval_define\`：在环境中定义变量
- \`eval_if\`：条件求值（注意只求值一个分支）
- \`eval_quote\`：返回未求值的数据
- \`eval_begin\`：顺序求值

### 步骤 4：实现函数调用

\`eval_application\`：求值函数和参数，然后调用。

### 步骤 5：运行测试

\`\`\`bash
cd tiny-interpreter
pytest learn/04-evaluator-basic/test_skeleton.py -v
\`\`\`

---

## 可视化：\`(+ 1 (* 2 3))\` 的求值过程

\`\`\`
eval(SExp[+, 1, SExp[*, 2, 3]])
│
├─ eval(Symbol(+))  → <内置加法函数>
├─ eval(Number(1))  → 1
├─ eval(SExp[*, 2, 3])
│  │
│  ├─ eval(Symbol(*))  → <内置乘法函数>
│  ├─ eval(Number(2))  → 2
│  └─ eval(Number(3))  → 3
│  └─ 调用 乘法(2, 3)  → 6
│
└─ 调用 加法(1, 6)  → 7
\`\`\`

求值是一个**树遍历**过程：从根节点开始，递归求值子节点，最后组合结果。

---

## 提示

如果卡住了，可以查看提示：

- [提示 1：eval() 的基本结构](hints/hint1.md)
- [提示 2：特殊形式的处理](hints/hint2.md)
- [提示 3：函数调用和 quote](hints/hint3.md)

---

## 深入思考

完成实现后，思考这些问题：

1. 为什么 \`if\` 必须是特殊形式而不能是普通函数？
2. \`quote\` 的存在说明了代码和数据之间的什么关系？
3. 如果没有 \`begin\`，能不能实现同样的功能？
4. 求值器的递归结构和 AST 的递归结构有什么对应关系？

---

## 下一步

现在我们的求值器可以处理基本表达式了。

但还缺少一个关键能力：**定义自己的函数**。

[进入模块 5：Lambda 与闭包 →](../05-lambda-closure/README.md)`,
    en: `# Module 4: Basic Evaluator

> "How are expression values computed?"

## Problem Introduction

We already have:
- **Lexer**: turns source code into tokens
- **Parser**: turns tokens into AST
- **Environment**: stores variable bindings

Now the question is: **how do we compute the value of an expression?**

\`\`\`lisp
(+ 1 (* 2 3))  ; result should be 7
\`\`\`

This looks simple, but think carefully:
- \`(* 2 3)\` needs to be evaluated to 6 first
- Then \`(+ 1 6)\` can be evaluated to 7
- Evaluation is **recursive**!

---

## Core Concepts

### Evaluation Rules for Different Node Types

| Node Type | Evaluation Rule | Example |
|-----------|----------------|---------|
| Number | Return value directly | \`42\` → \`42\` |
| Boolean | Return value directly | \`#t\` → \`True\` |
| Symbol | Look up in environment | \`x\` → \`env.get("x")\` |
| SExpression | Special form or function call | \`(+ 1 2)\` → \`3\` |

### Self-Evaluating Expressions

Numbers and booleans are "self-evaluating" — their value is themselves:

\`\`\`python
eval(Number(42))  → 42
eval(Boolean(True))  → True
\`\`\`

### Variable Lookup

Symbols need to be looked up in the environment:

\`\`\`python
eval(Symbol("x"))  → env.get("x")
\`\`\`

### Function Calls

S-expression evaluation is the most complex:

\`\`\`python
eval((+ 1 (* 2 3)))
# 1. Evaluate +  → get the addition function
# 2. Evaluate 1  → get 1
# 3. Evaluate (* 2 3)  → recursively evaluate, get 6
# 4. Call addition(1, 6)  → 7
\`\`\`

---

## Special Forms

Some S-expressions are not function calls but **special forms** — they have their own evaluation rules.

### define

\`\`\`lisp
(define x 42)
\`\`\`

This doesn't call a \`define\` function; it creates a binding in the environment.

### if

\`\`\`lisp
(if (< 1 2) 10 20)  ; → 10
\`\`\`

**Why is if a special form?** Because it must not evaluate all its arguments!

If \`if\` were a regular function, both the \`then\` and \`else\` branches would be evaluated. But \`if\` should only evaluate one branch.

### quote

\`\`\`lisp
(quote (+ 1 2))  ; → ["+", 1, 2] (not evaluated, returned as data)
\`\`\`

\`quote\` prevents evaluation and returns code as data.

### begin

\`\`\`lisp
(begin
  (define x 10)
  (define y 20)
  (+ x y))  ; → 30
\`\`\`

\`begin\` evaluates each expression in sequence and returns the value of the last one.

---

## Hands-on Implementation

### Step 1: Implement Basic eval() Dispatch

Handle the three simple cases first: Number, Boolean, Symbol.

### Step 2: Handle SExpression

Distinguish special forms (define, if, quote, begin) from regular function calls.

### Step 3: Implement Each Special Form

- \`eval_define\`: define variables in the environment
- \`eval_if\`: conditional evaluation (note: only evaluate one branch)
- \`eval_quote\`: return unevaluated data
- \`eval_begin\`: sequential evaluation

### Step 4: Implement Function Calls

\`eval_application\`: evaluate the function and arguments, then call.

### Step 5: Run Tests

\`\`\`bash
cd tiny-interpreter
pytest learn/04-evaluator-basic/test_skeleton.py -v
\`\`\`

---

## Visualization: Evaluating \`(+ 1 (* 2 3))\`

\`\`\`
eval(SExp[+, 1, SExp[*, 2, 3]])
│
├─ eval(Symbol(+))  → <built-in addition>
├─ eval(Number(1))  → 1
├─ eval(SExp[*, 2, 3])
│  │
│  ├─ eval(Symbol(*))  → <built-in multiplication>
│  ├─ eval(Number(2))  → 2
│  └─ eval(Number(3))  → 3
│  └─ call multiply(2, 3)  → 6
│
└─ call add(1, 6)  → 7
\`\`\`

Evaluation is a **tree traversal** process: start from the root, recursively evaluate child nodes, then combine results.

---

## Hints

If you get stuck, check the hints:

- [Hint 1: Basic eval() structure](hints/hint1.md)
- [Hint 2: Handling special forms](hints/hint2.md)
- [Hint 3: Function calls and quote](hints/hint3.md)

---

## Deep Thinking

After completing the implementation, think about these questions:

1. Why must \`if\` be a special form rather than a regular function?
2. What does the existence of \`quote\` tell us about the relationship between code and data?
3. Could we achieve the same functionality without \`begin\`?
4. What is the correspondence between the recursive structure of the evaluator and the recursive structure of the AST?

---

## Next Step

Now our evaluator can handle basic expressions.

But it's still missing a key capability: **defining your own functions**.

[Enter Module 5: Lambda and Closures →](../05-lambda-closure/README.md)`,
  },
  hints: [
    { level: 1, content: { zh: `# 提示 1：eval() 的基本结构

## Number、Boolean、Symbol 的处理

\`eval()\` 的核心是一个类型分发：

\`\`\`python
def eval(self, node: ASTNode, env: Environment) -> Any:
    # 1. 数字：直接返回值
    if isinstance(node, Number):
        return node.value

    # 2. 布尔值：直接返回值
    if isinstance(node, Boolean):
        return node.value

    # 3. 符号：在环境中查找
    if isinstance(node, Symbol):
        return env.get(node.name)

    # 4. S-表达式：见提示 2
    if isinstance(node, SExpression):
        # ...
        pass
\`\`\`

## 关键洞察

前三种情况非常简单：
- Number 和 Boolean 是"自求值"的
- Symbol 就是查环境

复杂的部分在 SExpression——见提示 2。`, en: `# Hint 1: Basic eval() Structure

## Handling Number, Boolean, Symbol

The core of \`eval()\` is type dispatch:

\`\`\`python
def eval(self, node: ASTNode, env: Environment) -> Any:
    # 1. Number: return value directly
    if isinstance(node, Number):
        return node.value

    # 2. Boolean: return value directly
    if isinstance(node, Boolean):
        return node.value

    # 3. Symbol: look up in environment
    if isinstance(node, Symbol):
        return env.get(node.name)

    # 4. SExpression: see hint 2
    if isinstance(node, SExpression):
        # ...
        pass
\`\`\`

## Key Insight

The first three cases are very simple:
- Number and Boolean are "self-evaluating"
- Symbol is just an environment lookup

The complex part is SExpression — see hint 2.` } },
    { level: 2, content: { zh: `# 提示 2：S-表达式和特殊形式

## SExpression 的处理

\`\`\`python
if isinstance(node, SExpression):
    elements = node.elements

    # 空列表
    if len(elements) == 0:
        return []

    first = elements[0]

    # 检查是否是特殊形式
    if isinstance(first, Symbol):
        if first.name == "define":
            return self.eval_define(elements[1:], env)
        elif first.name == "if":
            return self.eval_if(elements[1:], env)
        elif first.name == "quote":
            return self.eval_quote(elements[1:])
        elif first.name == "begin":
            return self.eval_begin(elements[1:], env)

    # 不是特殊形式，就是函数调用
    return self.eval_application(elements, env)
\`\`\`

## eval_define 的实现

\`\`\`python
def eval_define(self, args, env):
    if len(args) != 2:
        raise EvaluatorError("define requires 2 arguments")
    if not isinstance(args[0], Symbol):
        raise EvaluatorError("define first arg must be Symbol")
    value = self.eval(args[1], env)
    env.define(args[0].name, value)
    return None
\`\`\`

## eval_if 的实现

\`\`\`python
def eval_if(self, args, env):
    if len(args) != 3:
        raise EvaluatorError("if requires 3 arguments")
    condition = self.eval(args[0], env)
    if condition:
        return self.eval(args[1], env)
    else:
        return self.eval(args[2], env)
\`\`\`

注意：只求值需要的分支！这就是 if 必须是特殊形式的原因。`, en: `# Hint 2: S-Expressions and Special Forms

## Handling SExpression

\`\`\`python
if isinstance(node, SExpression):
    elements = node.elements

    # Empty list
    if len(elements) == 0:
        return []

    first = elements[0]

    # Check if it's a special form
    if isinstance(first, Symbol):
        if first.name == "define":
            return self.eval_define(elements[1:], env)
        elif first.name == "if":
            return self.eval_if(elements[1:], env)
        elif first.name == "quote":
            return self.eval_quote(elements[1:])
        elif first.name == "begin":
            return self.eval_begin(elements[1:], env)

    # Not a special form, it's a function call
    return self.eval_application(elements, env)
\`\`\`

## eval_define Implementation

\`\`\`python
def eval_define(self, args, env):
    if len(args) != 2:
        raise EvaluatorError("define requires 2 arguments")
    if not isinstance(args[0], Symbol):
        raise EvaluatorError("define first arg must be Symbol")
    value = self.eval(args[1], env)
    env.define(args[0].name, value)
    return None
\`\`\`

## eval_if Implementation

\`\`\`python
def eval_if(self, args, env):
    if len(args) != 3:
        raise EvaluatorError("if requires 3 arguments")
    condition = self.eval(args[0], env)
    if condition:
        return self.eval(args[1], env)
    else:
        return self.eval(args[2], env)
\`\`\`

Note: only evaluate the needed branch! This is why if must be a special form.` } },
    { level: 3, content: { zh: `# 提示 3：函数调用、quote 和 begin

## eval_application 的实现

\`\`\`python
def eval_application(self, elements, env):
    # 1. 求值函数（第一个元素）
    func = self.eval(elements[0], env)

    # 2. 求值所有参数
    args = [self.eval(arg, env) for arg in elements[1:]]

    # 3. 调用函数
    return func(*args)
\`\`\`

关键：先求值函数，再求值参数，最后调用。

## eval_quote 的实现

\`\`\`python
def eval_quote(self, args):
    if len(args) != 1:
        raise EvaluatorError("quote requires 1 argument")
    return self.ast_to_value(args[0])
\`\`\`

\`quote\` 不求值参数，而是把 AST 转换为数据。

## eval_begin 的实现

\`\`\`python
def eval_begin(self, args, env):
    result = None
    for expr in args:
        result = self.eval(expr, env)
    return result
\`\`\`

\`begin\` 依次求值每个表达式，返回最后一个的值。

## 完整实现

如果你还是卡住了，可以参考 \`src/tiny_interpreter/evaluator.py\` 中的完整实现。`, en: `# Hint 3: Function Calls, quote, and begin

## eval_application Implementation

\`\`\`python
def eval_application(self, elements, env):
    # 1. Evaluate the function (first element)
    func = self.eval(elements[0], env)

    # 2. Evaluate all arguments
    args = [self.eval(arg, env) for arg in elements[1:]]

    # 3. Call the function
    return func(*args)
\`\`\`

Key: evaluate the function first, then the arguments, then call.

## eval_quote Implementation

\`\`\`python
def eval_quote(self, args):
    if len(args) != 1:
        raise EvaluatorError("quote requires 1 argument")
    return self.ast_to_value(args[0])
\`\`\`

\`quote\` doesn't evaluate its argument; it converts the AST to data.

## eval_begin Implementation

\`\`\`python
def eval_begin(self, args, env):
    result = None
    for expr in args:
        result = self.eval(expr, env)
    return result
\`\`\`

\`begin\` evaluates each expression in sequence and returns the value of the last one.

## Complete Implementation

If you're still stuck, you can refer to the complete implementation in \`src/tiny_interpreter/evaluator.py\`.` } },
  ],
  guidance: [
    {
      question: {
        zh: "`eval()` 的基本结构是什么？需要处理几种节点类型？",
        en: "What's the basic structure of `eval()`? How many node types need handling?"
      }
    },
    {
      question: {
        zh: "Number 和 Boolean 为什么叫'自求值'？它们的求值规则是什么？",
        en: "Why are Number and Boolean called 'self-evaluating'? What's their evaluation rule?"
      }
    },
    {
      question: {
        zh: "Symbol 的求值需要什么？在哪里查找它的值？",
        en: "What does Symbol evaluation need? Where do we look up its value?"
      }
    },
    {
      question: {
        zh: "为什么 `if` 必须是特殊形式而不能是普通函数？",
        en: "Why must `if` be a special form rather than a regular function?"
      }
    },
    {
      question: {
        zh: "`eval_application()` 的三个步骤是什么？顺序重要吗？",
        en: "What are the three steps in `eval_application()`? Does order matter?"
      }
    },
    {
      question: {
        zh: "`quote` 做什么？它为什么不求值参数？",
        en: "What does `quote` do? Why doesn't it evaluate its argument?"
      }
    }
  ],
};