import type { LearningModule } from "../types";

export const module05EvaluatorLambda: LearningModule = {
  id: "05-evaluator-lambda",
  index: 5,
  slug: "evaluator-lambda",
  hasCode: true,
  skeleton: `"""Lambda 与闭包骨架代码。

你的任务是实现标记为 TODO 的方法。

运行测试：
    pytest learn/05-evaluator-lambda/test_skeleton.py -v

这个模块在模块 4 的基础上添加 lambda 和闭包支持。
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


class Closure:
    """闭包：捕获函数定义时的环境。

    TODO: 理解这个类

    闭包是函数的运行时表示，包含：
    - params: 参数名列表
    - body: 函数体（AST 节点列表）
    - env: 定义时的环境（这是闭包的关键！）

    当闭包被调用时：
    1. 创建新环境，parent 指向 self.env
    2. 在新环境中绑定参数
    3. 在新环境中执行函数体
    """

    def __init__(self, params: List[str], body: List[ASTNode], env: Environment):
        """创建闭包。

        Args:
            params: 参数名列表，如 ['x', 'y']
            body: 函数体，AST 节点列表
            env: 定义时的环境（会被闭包"捕获"）
        """
        self.params = params
        self.body = body
        self.env = env  # 这是闭包的魔力所在！

    def __repr__(self):
        return f"<closure {self.params}>"


class Evaluator:
    """求值器：支持 lambda 和闭包。"""

    def __init__(self):
        """初始化求值器。"""
        self.global_env = self.create_global_environment()

    def create_global_environment(self) -> Environment:
        """创建全局环境。"""
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
        """求值 AST 节点。"""
        if isinstance(node, Number):
            return node.value

        if isinstance(node, Boolean):
            return node.value

        if isinstance(node, Symbol):
            return env.get(node.name)

        if isinstance(node, SExpression):
            if len(node.elements) == 0:
                return []

            first = node.elements[0]

            if isinstance(first, Symbol):
                if first.name == 'define':
                    return self.eval_define(node.elements[1:], env)

                if first.name == 'lambda':
                    return self.eval_lambda(node.elements[1:], env)

                if first.name == 'if':
                    return self.eval_if(node.elements[1:], env)

                if first.name == 'quote':
                    return self.eval_quote(node.elements[1:])

                if first.name == 'begin':
                    return self.eval_begin(node.elements[1:], env)

            return self.eval_application(node.elements, env)

        raise EvaluatorError(f"Unknown node type: {type(node)}")

    def eval_define(self, args: List[ASTNode], env: Environment) -> None:
        """求值 define。"""
        if len(args) != 2:
            raise EvaluatorError(f"define expects 2 arguments, got {len(args)}")

        name_node = args[0]
        if not isinstance(name_node, Symbol):
            raise EvaluatorError("define expects a symbol as first argument")

        value = self.eval(args[1], env)
        env.define(name_node.name, value)
        return None

    def eval_lambda(self, args: List[ASTNode], env: Environment) -> Closure:
        """求值 lambda 表达式，创建闭包。

        TODO: 实现这个方法

        形式：(lambda (param1 param2 ...) body1 body2 ...)

        Args:
            args: lambda 的参数，[params-list, body1, body2, ...]
            env: 当前环境

        Returns:
            Closure 对象

        步骤：
        1. 检查参数数量至少为 2（参数列表 + 至少一个 body）
        2. 提取参数列表（第一个元素，应该是 SExpression）
        3. 验证参数列表中的每个元素都是 Symbol
        4. 提取参数名列表
        5. 提取函数体（剩余的元素）
        6. 创建 Closure，捕获当前环境 env

        关键：Closure 的 env 参数应该是当前的 env！
        这就是闭包"捕获"环境的地方。
        """
        # TODO: 实现
        pass

    def eval_if(self, args: List[ASTNode], env: Environment) -> Any:
        """求值 if。"""
        if len(args) != 3:
            raise EvaluatorError(f"if expects 3 arguments, got {len(args)}")

        condition = self.eval(args[0], env)
        if condition:
            return self.eval(args[1], env)
        else:
            return self.eval(args[2], env)

    def eval_quote(self, args: List[ASTNode]) -> Any:
        """求值 quote。"""
        if len(args) != 1:
            raise EvaluatorError(f"quote expects 1 argument, got {len(args)}")
        return self.ast_to_value(args[0])

    def eval_begin(self, args: List[ASTNode], env: Environment) -> Any:
        """求值 begin。"""
        result = None
        for expr in args:
            result = self.eval(expr, env)
        return result

    def eval_application(self, elements: List[ASTNode], env: Environment) -> Any:
        """求值函数调用。

        TODO: 修改这个方法以支持闭包调用

        形式：(func arg1 arg2 ...)

        Args:
            elements: [func, arg1, arg2, ...]
            env: 当前环境

        Returns:
            函数调用结果

        步骤：
        1. 求值函数
        2. 求值所有参数
        3. 如果函数是内置函数（callable 但不是 Closure），直接调用
        4. 如果函数是 Closure：
           a. 检查参数数量是否匹配
           b. 创建新环境，parent 是 closure.env（不是当前 env！）
           c. 在新环境中绑定参数
           d. 在新环境中依次求值函数体
           e. 返回最后一个表达式的值

        关键：新环境的 parent 是 closure.env，这实现了词法作用域！
        """
        # 1. 求值函数
        func = self.eval(elements[0], env)

        # 2. 求值参数
        args = [self.eval(arg, env) for arg in elements[1:]]

        # 3. 内置函数
        if callable(func) and not isinstance(func, Closure):
            return func(*args)

        # 4. 闭包调用
        if isinstance(func, Closure):
            # TODO: 实现闭包调用
            # a. 检查参数数量
            # b. 创建新环境，parent 是 func.env
            # c. 绑定参数
            # d. 执行函数体
            # e. 返回结果
            pass

        raise EvaluatorError(f"Not a function: {func}")

    def ast_to_value(self, node: ASTNode) -> Any:
        """将 AST 转换为值。"""
        if isinstance(node, Number):
            return node.value
        if isinstance(node, Boolean):
            return node.value
        if isinstance(node, Symbol):
            return node.name
        if isinstance(node, SExpression):
            return [self.ast_to_value(elem) for elem in node.elements]
        return node

    def run(self, source: str) -> Any:
        """解析并执行源代码。"""
        ast_nodes = parse(source)
        result = None
        for node in ast_nodes:
            result = self.eval(node, self.global_env)
        return result`,
  testCode: `"""Lambda 与闭包测试。

运行测试：
    pytest learn/05-evaluator-lambda/test_skeleton.py -v
"""

import pytest
from skeleton import Evaluator, Closure, EvaluatorError


@pytest.fixture
def evaluator():
    """创建求值器实例。"""
    return Evaluator()


class TestLambdaBasics:
    """Lambda 基础测试。"""

    def test_lambda_creates_closure(self, evaluator):
        """测试 lambda 创建闭包。"""
        result = evaluator.run("(lambda (x) x)")
        assert isinstance(result, Closure)

    def test_lambda_params(self, evaluator):
        """测试闭包的参数。"""
        result = evaluator.run("(lambda (x y) (+ x y))")
        assert result.params == ['x', 'y']

    def test_simple_lambda_call(self, evaluator):
        """测试简单的 lambda 调用。"""
        result = evaluator.run("((lambda (x) x) 42)")
        assert result == 42

    def test_lambda_with_arithmetic(self, evaluator):
        """测试带算术的 lambda。"""
        result = evaluator.run("((lambda (x) (* x x)) 5)")
        assert result == 25

    def test_lambda_multiple_params(self, evaluator):
        """测试多参数 lambda。"""
        result = evaluator.run("((lambda (x y) (+ x y)) 3 4)")
        assert result == 7


class TestDefineFunction:
    """定义函数测试。"""

    def test_define_and_call(self, evaluator):
        """测试定义并调用函数。"""
        evaluator.run("(define square (lambda (x) (* x x)))")
        result = evaluator.run("(square 5)")
        assert result == 25

    def test_define_multiple_functions(self, evaluator):
        """测试定义多个函数。"""
        evaluator.run("(define double (lambda (x) (* x 2)))")
        evaluator.run("(define triple (lambda (x) (* x 3)))")
        assert evaluator.run("(double 5)") == 10
        assert evaluator.run("(triple 5)") == 15

    def test_function_using_global(self, evaluator):
        """测试函数使用全局变量。"""
        evaluator.run("(define y 10)")
        evaluator.run("(define add-y (lambda (x) (+ x y)))")
        result = evaluator.run("(add-y 5)")
        assert result == 15


class TestClosure:
    """闭包测试 - 这是最重要的部分！"""

    def test_closure_captures_environment(self, evaluator):
        """测试闭包捕获环境。"""
        code = """
        (define make-adder
          (lambda (x)
            (lambda (y) (+ x y))))
        """
        evaluator.run(code)

        evaluator.run("(define add5 (make-adder 5))")
        result = evaluator.run("(add5 3)")
        assert result == 8

    def test_multiple_closures_independent(self, evaluator):
        """测试多个闭包相互独立。"""
        code = """
        (define make-adder
          (lambda (x)
            (lambda (y) (+ x y))))
        """
        evaluator.run(code)

        evaluator.run("(define add5 (make-adder 5))")
        evaluator.run("(define add10 (make-adder 10))")

        assert evaluator.run("(add5 1)") == 6
        assert evaluator.run("(add10 1)") == 11

    def test_closure_lexical_scope(self, evaluator):
        """测试闭包使用词法作用域。"""
        code = """
        (define x 10)
        (define f (lambda () x))
        (define g
          (lambda ()
            (define x 20)
            (f)))
        """
        evaluator.run(code)

        # 词法作用域：f 看到的是定义时的 x=10，不是调用时的 x=20
        result = evaluator.run("(g)")
        assert result == 10


class TestRecursion:
    """递归测试。"""

    def test_factorial(self, evaluator):
        """测试阶乘。"""
        code = """
        (define factorial
          (lambda (n)
            (if (= n 0)
                1
                (* n (factorial (- n 1))))))
        """
        evaluator.run(code)

        assert evaluator.run("(factorial 0)") == 1
        assert evaluator.run("(factorial 1)") == 1
        assert evaluator.run("(factorial 5)") == 120

    def test_fibonacci(self, evaluator):
        """测试斐波那契。"""
        code = """
        (define fib
          (lambda (n)
            (if (< n 2)
                n
                (+ (fib (- n 1)) (fib (- n 2))))))
        """
        evaluator.run(code)

        assert evaluator.run("(fib 0)") == 0
        assert evaluator.run("(fib 1)") == 1
        assert evaluator.run("(fib 10)") == 55


class TestHigherOrderFunctions:
    """高阶函数测试。"""

    def test_apply_twice(self, evaluator):
        """测试 apply-twice。"""
        code = """
        (define apply-twice
          (lambda (f x)
            (f (f x))))
        (define add1 (lambda (x) (+ x 1)))
        """
        evaluator.run(code)

        result = evaluator.run("(apply-twice add1 5)")
        assert result == 7

    def test_compose(self, evaluator):
        """测试函数组合。"""
        code = """
        (define compose
          (lambda (f g)
            (lambda (x) (f (g x)))))
        (define double (lambda (x) (* x 2)))
        (define add1 (lambda (x) (+ x 1)))
        (define double-then-add1 (compose add1 double))
        """
        evaluator.run(code)

        result = evaluator.run("(double-then-add1 5)")
        assert result == 11  # (5 * 2) + 1


class TestMultipleBodyExpressions:
    """多表达式函数体测试。"""

    def test_lambda_multiple_body(self, evaluator):
        """测试 lambda 有多个 body 表达式。"""
        code = """
        (define f
          (lambda (x)
            (define y (* x 2))
            (+ x y)))
        """
        evaluator.run(code)

        result = evaluator.run("(f 5)")
        assert result == 15  # 5 + 10


class TestErrors:
    """错误处理测试。"""

    def test_wrong_argument_count(self, evaluator):
        """测试参数数量错误。"""
        evaluator.run("(define f (lambda (x y) (+ x y)))")

        with pytest.raises(EvaluatorError):
            evaluator.run("(f 1)")  # 缺少参数

    def test_call_non_function(self, evaluator):
        """测试调用非函数。"""
        evaluator.run("(define x 42)")

        with pytest.raises(EvaluatorError):
            evaluator.run("(x 1)")  # 42 不是函数`,
  readme: {
    zh: `# 模块 5：Lambda 与闭包 (Lambda & Closures)

> "这是整个项目的顿悟时刻！"

## 问题引入

我们已经有了一个能处理基本表达式的求值器。但它还不能定义自己的函数。

想象一下，你想写一个"加法器工厂"：

\`\`\`lisp
(define make-adder
  (lambda (x)
    (lambda (y) (+ x y))))

(define add5 (make-adder 5))
(add5 3)  ; → 8
\`\`\`

这段代码做了什么？
1. \`make-adder\` 接受一个数 \`x\`，返回一个新函数
2. 返回的函数接受 \`y\`，返回 \`x + y\`
3. \`(make-adder 5)\` 返回一个"记住了 x=5"的函数
4. \`(add5 3)\` 计算 \`5 + 3 = 8\`

关键问题：**返回的函数怎么"记住" x=5？**

当 \`make-adder\` 执行完毕后，它的局部变量 \`x\` 应该已经消失了。但 \`add5\` 还能用 \`x\`！

这就是**闭包**的魔力。

---

## 核心概念

### 什么是闭包？

闭包 = 函数代码 + 捕获的环境

\`\`\`
┌─────────────────────────────────┐
│           Closure               │
│                                 │
│  ┌───────────┐  ┌───────────┐  │
│  │  params:  │  │   env:    │  │
│  │  ['y']    │  │  ┌─────┐  │  │
│  └───────────┘  │  │x: 5 │  │  │
│                 │  └──┬──┘  │  │
│  ┌───────────┐  │     │     │  │
│  │  body:    │  │  ┌──▼──┐  │  │
│  │ (+ x y)   │  │  │全局 │  │  │
│  └───────────┘  │  │环境 │  │  │
│                 │  └─────┘  │  │
│                 └───────────┘  │
└─────────────────────────────────┘
\`\`\`

闭包不只是一段代码，它还"记住"了自己出生时的环境。

### 关键洞察

闭包捕获的是**定义时**的环境，不是**调用时**的环境。

\`\`\`lisp
(define x 10)
(define f (lambda () x))    ; f 捕获了 x=10 的环境

(define g
  (lambda ()
    (define x 20)           ; 这个 x 是 g 的局部变量
    (f)))                   ; f 看到的还是 x=10！

(g)  ; → 10，不是 20！
\`\`\`

这就是**词法作用域**（lexical scoping）：函数看到的变量取决于它在代码中的位置（定义时），而不是它被调用的位置（运行时）。

### make-adder 的执行过程

让我们一步步跟踪 \`make-adder\` 的执行：

\`\`\`
第 1 步：定义 make-adder
全局环境: { make-adder: <closure (x) ...> }

第 2 步：调用 (make-adder 5)
├─ 创建新环境 E1: { x: 5 }, parent → 全局环境
├─ 在 E1 中求值 (lambda (y) (+ x y))
└─ 返回 <closure params=['y'] body=(+ x y) env=E1>
                                          ^^^
                                    闭包捕获了 E1！

第 3 步：add5 = <closure params=['y'] body=(+ x y) env=E1>
全局环境: { make-adder: ..., add5: <closure> }

第 4 步：调用 (add5 3)
├─ 创建新环境 E2: { y: 3 }, parent → E1（不是全局环境！）
├─ 在 E2 中求值 (+ x y)
│  ├─ x → 在 E2 中找不到 → 在 E1 中找到 x=5
│  └─ y → 在 E2 中找到 y=3
└─ 返回 5 + 3 = 8
\`\`\`

---

## 动手实现

### 步骤 1：理解 Closure 类

先阅读 \`Closure\` 类的代码。它有三个属性：
- \`params\`：参数名列表
- \`body\`：函数体（AST 节点列表）
- \`env\`：定义时的环境

### 步骤 2：实现 eval_lambda()

\`eval_lambda\` 需要：
1. 提取参数列表
2. 提取函数体
3. 创建 Closure，捕获当前环境

### 步骤 3：实现闭包调用

在 \`eval_application\` 中添加闭包调用逻辑：
1. 检查参数数量
2. 创建新环境（parent 是 closure.env！）
3. 绑定参数
4. 执行函数体

### 步骤 4：运行测试

\`\`\`bash
cd tiny-interpreter
pytest learn/05-evaluator-lambda/test_skeleton.py -v
\`\`\`

---

## 提示

如果卡住了，可以查看提示：

- [提示 1：理解 Closure 类](hints/hint1.md)
- [提示 2：实现 eval_lambda()](hints/hint2.md)
- [提示 3：实现闭包调用](hints/hint3.md)

---

## 深入思考

完成实现后，思考这些问题：

1. 如果闭包捕获的是调用时的环境而不是定义时的环境，会发生什么？（提示：这叫动态作用域）
2. 为什么 \`eval_application\` 中创建新环境时，parent 必须是 \`closure.env\` 而不是当前的 \`env\`？
3. 闭包和对象有什么关系？（提示："闭包是穷人的对象，对象是穷人的闭包"）
4. 递归为什么能工作？\`factorial\` 函数在调用自己时，是怎么找到自己的？

---

## 恭喜！

如果你通过了所有测试，你已经实现了一个支持闭包的解释器！

这是整个项目最重要的顿悟时刻：
- 你理解了函数不只是代码，还包含环境
- 你理解了词法作用域的实现原理
- 你理解了高阶函数和函数式编程的基础

这些概念是所有现代编程语言的基石。JavaScript 的闭包、Python 的闭包、Rust 的闭包——它们的核心原理都是一样的。

---

## 下一步

[进入模块 6：高级特性 →](../06-advanced/README.md)`,
    en: `# Module 5: Lambda & Closures

> "This is the aha moment of the entire project!"

## Problem Introduction

We already have an evaluator that can handle basic expressions. But it can't define its own functions yet.

Imagine you want to write an "adder factory":

\`\`\`lisp
(define make-adder
  (lambda (x)
    (lambda (y) (+ x y))))

(define add5 (make-adder 5))
(add5 3)  ; → 8
\`\`\`

What does this code do?
1. \`make-adder\` takes a number \`x\` and returns a new function
2. The returned function takes \`y\` and returns \`x + y\`
3. \`(make-adder 5)\` returns a function that "remembers x=5"
4. \`(add5 3)\` computes \`5 + 3 = 8\`

The key question: **how does the returned function "remember" x=5?**

When \`make-adder\` finishes executing, its local variable \`x\` should have disappeared. But \`add5\` can still use \`x\`!

This is the magic of **closures**.

---

## Core Concepts

### What is a Closure?

Closure = function code + captured environment

\`\`\`
┌─────────────────────────────────┐
│           Closure               │
│                                 │
│  ┌───────────┐  ┌───────────┐  │
│  │  params:  │  │   env:    │  │
│  │  ['y']    │  │  ┌─────┐  │  │
│  └───────────┘  │  │x: 5 │  │  │
│                 │  └──┬──┘  │  │
│  ┌───────────┐  │     │     │  │
│  │  body:    │  │  ┌──▼──┐  │  │
│  │ (+ x y)   │  │  │glob │  │  │
│  └───────────┘  │  │ env │  │  │
│                 │  └─────┘  │  │
│                 └───────────┘  │
└─────────────────────────────────┘
\`\`\`

A closure is not just a piece of code — it also "remembers" the environment where it was born.

### Key Insight

Closures capture the **defining** environment, not the **calling** environment.

\`\`\`lisp
(define x 10)
(define f (lambda () x))    ; f captures the environment where x=10

(define g
  (lambda ()
    (define x 20)           ; this x is local to g
    (f)))                   ; f still sees x=10!

(g)  ; → 10, not 20!
\`\`\`

This is **lexical scoping**: the variables a function sees depend on where it is in the code (definition time), not where it is called (runtime).

### Step-by-Step Execution of make-adder

Let's trace the execution of \`make-adder\` step by step:

\`\`\`
Step 1: Define make-adder
Global env: { make-adder: <closure (x) ...> }

Step 2: Call (make-adder 5)
├─ Create new env E1: { x: 5 }, parent → global env
├─ Evaluate (lambda (y) (+ x y)) in E1
└─ Return <closure params=['y'] body=(+ x y) env=E1>
                                          ^^^
                                    closure captures E1!

Step 3: add5 = <closure params=['y'] body=(+ x y) env=E1>
Global env: { make-adder: ..., add5: <closure> }

Step 4: Call (add5 3)
├─ Create new env E2: { y: 3 }, parent → E1 (not global env!)
├─ Evaluate (+ x y) in E2
│  ├─ x → not in E2 → found in E1: x=5
│  └─ y → found in E2: y=3
└─ Return 5 + 3 = 8
\`\`\`

---

## Hands-on Implementation

### Step 1: Understand the Closure Class

Read the \`Closure\` class code first. It has three attributes:
- \`params\`: list of parameter names
- \`body\`: function body (list of AST nodes)
- \`env\`: the environment at definition time

### Step 2: Implement eval_lambda()

\`eval_lambda\` needs to:
1. Extract the parameter list
2. Extract the function body
3. Create a Closure, capturing the current environment

### Step 3: Implement Closure Invocation

Add closure invocation logic in \`eval_application\`:
1. Check argument count
2. Create a new environment (parent is closure.env!)
3. Bind parameters
4. Execute the function body

### Step 4: Run Tests

\`\`\`bash
cd tiny-interpreter
pytest learn/05-evaluator-lambda/test_skeleton.py -v
\`\`\`

---

## Hints

If you get stuck, check the hints:

- [Hint 1: Understanding the Closure class](hints/hint1.md)
- [Hint 2: Implementing eval_lambda()](hints/hint2.md)
- [Hint 3: Implementing closure invocation](hints/hint3.md)

---

## Deep Thinking

After completing the implementation, think about these questions:

1. What would happen if closures captured the calling environment instead of the defining environment? (Hint: that's called dynamic scoping)
2. Why must the parent of the new environment in \`eval_application\` be \`closure.env\` rather than the current \`env\`?
3. What is the relationship between closures and objects? (Hint: "closures are a poor man's objects, objects are a poor man's closures")
4. Why does recursion work? When \`factorial\` calls itself, how does it find itself?

---

## Congratulations!

If you passed all the tests, you've implemented an interpreter with closure support!

This is the most important aha moment of the entire project:
- You understand that functions are not just code — they include an environment
- You understand how lexical scoping is implemented
- You understand the foundation of higher-order functions and functional programming

These concepts are the cornerstone of all modern programming languages. JavaScript closures, Python closures, Rust closures — their core principles are all the same.

---

## Next Step

[Enter Module 6: Advanced Features →](../06-advanced/README.md)`,
  },
  hints: [
    { level: 1, content: { zh: `# 提示 1：理解 Closure 类

## Closure 的三个属性

\`\`\`python
class Closure:
    def __init__(self, params, body, env):
        self.params = params  # 参数名列表，如 ['x', 'y']
        self.body = body      # 函数体，AST 节点列表
        self.env = env        # 定义时的环境 ← 这是关键！
\`\`\`

## 为什么 env 是关键属性？

考虑这个例子：

\`\`\`lisp
(define make-adder
  (lambda (x)
    (lambda (y) (+ x y))))

(define add5 (make-adder 5))
\`\`\`

当 \`(make-adder 5)\` 执行时：
1. 创建环境 E1: { x: 5 }
2. 在 E1 中求值 \`(lambda (y) (+ x y))\`
3. 创建 Closure: params=['y'], body=(+ x y), **env=E1**

\`env=E1\` 就是闭包"记住" x=5 的方式！

当 \`(add5 3)\` 执行时：
1. 创建环境 E2: { y: 3 }, **parent=E1**（来自 closure.env）
2. 在 E2 中求值 (+ x y)
3. x 在 E2 中找不到 → 在 E1 中找到 x=5

## 关键洞察

Closure 的 \`env\` 属性让函数"记住"了它出生时的环境。这就是闭包的全部秘密。`, en: `# Hint 1: Understanding the Closure Class

## The Three Attributes of Closure

\`\`\`python
class Closure:
    def __init__(self, params, body, env):
        self.params = params  # parameter name list, e.g. ['x', 'y']
        self.body = body      # function body, list of AST nodes
        self.env = env        # environment at definition time ← this is the key!
\`\`\`

## Why is env the Key Attribute?

Consider this example:

\`\`\`lisp
(define make-adder
  (lambda (x)
    (lambda (y) (+ x y))))

(define add5 (make-adder 5))
\`\`\`

When \`(make-adder 5)\` executes:
1. Create environment E1: { x: 5 }
2. Evaluate \`(lambda (y) (+ x y))\` in E1
3. Create Closure: params=['y'], body=(+ x y), **env=E1**

\`env=E1\` is how the closure "remembers" x=5!

When \`(add5 3)\` executes:
1. Create environment E2: { y: 3 }, **parent=E1** (from closure.env)
2. Evaluate (+ x y) in E2
3. x not found in E2 → found in E1: x=5

## Key Insight

The \`env\` attribute of Closure lets the function "remember" the environment where it was born. That's the entire secret of closures.` } },
    { level: 2, content: { zh: `# 提示 2：实现 eval_lambda()

## 完整实现

\`\`\`python
def eval_lambda(self, args, env):
    # 1. 检查参数数量
    if len(args) < 2:
        raise EvaluatorError(
            f"lambda expects at least 2 arguments, got {len(args)}")

    # 2. 提取参数列表
    params_node = args[0]
    if not isinstance(params_node, SExpression):
        raise EvaluatorError("lambda params must be a list")

    # 3. 验证每个参数都是 Symbol
    for p in params_node.elements:
        if not isinstance(p, Symbol):
            raise EvaluatorError(f"lambda param must be symbol, got {type(p)}")

    # 4. 提取参数名
    param_names = [p.name for p in params_node.elements]

    # 5. 提取函数体
    body = list(args[1:])

    # 6. 创建闭包，捕获当前环境！
    return Closure(param_names, body, env)
\`\`\`

## 关键点

- 参数列表是第一个元素（一个 SExpression）
- 函数体是剩余的所有元素（可以有多个）
- **env 参数就是当前的 env**——这就是"捕获环境"的实现！`, en: `# Hint 2: Implementing eval_lambda()

## Complete Implementation

\`\`\`python
def eval_lambda(self, args, env):
    # 1. Check argument count
    if len(args) < 2:
        raise EvaluatorError(
            f"lambda expects at least 2 arguments, got {len(args)}")

    # 2. Extract parameter list
    params_node = args[0]
    if not isinstance(params_node, SExpression):
        raise EvaluatorError("lambda params must be a list")

    # 3. Verify each parameter is a Symbol
    for p in params_node.elements:
        if not isinstance(p, Symbol):
            raise EvaluatorError(f"lambda param must be symbol, got {type(p)}")

    # 4. Extract parameter names
    param_names = [p.name for p in params_node.elements]

    # 5. Extract function body
    body = list(args[1:])

    # 6. Create closure, capturing the current environment!
    return Closure(param_names, body, env)
\`\`\`

## Key Points

- The parameter list is the first element (an SExpression)
- The function body is all remaining elements (can be multiple)
- **The env parameter is the current env** — this is how "capturing the environment" is implemented!` } },
    { level: 3, content: { zh: `# 提示 3：实现闭包调用

## eval_application 中的闭包调用

\`\`\`python
# 4. 闭包调用
if isinstance(func, Closure):
    # a. 检查参数数量
    if len(args) != len(func.params):
        raise EvaluatorError(
            f"Expected {len(func.params)} arguments, got {len(args)}")

    # b. 创建新环境，parent 是 func.env（不是 env！）
    call_env = Environment(func.env)

    # c. 绑定参数
    for param, arg in zip(func.params, args):
        call_env.define(param, arg)

    # d. 执行函数体
    result = None
    for expr in func.body:
        result = self.eval(expr, call_env)

    # e. 返回结果
    return result
\`\`\`

## 最关键的一行

\`\`\`python
call_env = Environment(func.env)  # ← 这一行决定了一切！
\`\`\`

为什么是 \`func.env\` 而不是 \`env\`？

- \`func.env\` = 函数**定义时**的环境 → **词法作用域**
- \`env\` = 函数**调用时**的环境 → 动态作用域

如果你用 \`Environment(env)\`，闭包测试会失败！

试试把 \`func.env\` 改成 \`env\`，看看 \`test_closure_lexical_scope\` 会怎样——这是理解词法作用域最好的方式。`, en: `# Hint 3: Implementing Closure Invocation

## Closure Invocation in eval_application

\`\`\`python
# 4. Closure invocation
if isinstance(func, Closure):
    # a. Check argument count
    if len(args) != len(func.params):
        raise EvaluatorError(
            f"Expected {len(func.params)} arguments, got {len(args)}")

    # b. Create new environment, parent is func.env (not env!)
    call_env = Environment(func.env)

    # c. Bind parameters
    for param, arg in zip(func.params, args):
        call_env.define(param, arg)

    # d. Execute function body
    result = None
    for expr in func.body:
        result = self.eval(expr, call_env)

    # e. Return result
    return result
\`\`\`

## The Most Critical Line

\`\`\`python
call_env = Environment(func.env)  # ← this line decides everything!
\`\`\`

Why \`func.env\` and not \`env\`?

- \`func.env\` = the environment when the function was **defined** → **lexical scoping**
- \`env\` = the environment when the function is **called** → dynamic scoping

If you use \`Environment(env)\`, the closure tests will fail!

Try changing \`func.env\` to \`env\` and see what happens with \`test_closure_lexical_scope\` — this is the best way to understand lexical scoping.` } },
  ],
};
