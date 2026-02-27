import type { LearningModule } from "../types";

export const module03Environment: LearningModule = {
  id: "03-environment",
  index: 3,
  slug: "environment",
  hasCode: true,
  vizType: "environment",
  diagnostics: [
    { pattern: "_allFail", message: { zh: "`define()` 最简单——把 name→value 存到 `self.bindings` 字典里", en: "`define()` is the simplest — store name→value in `self.bindings` dict" } },
    { pattern: ["test_get_from_parent", "test_child_shadows_parent", "test_deep_nesting"], message: { zh: "`get()` 在当前环境找不到时，有没有递归查找 `self.parent`？", en: "When `get()` can't find it locally, does it recursively search `self.parent`?" } },
    { pattern: ["test_set_local", "test_set_in_parent", "test_set_does_not_create"], message: { zh: "`set()` 和 `define()` 不同——它要修改已有的变量，不是创建新的", en: "`set()` differs from `define()` — it modifies existing variables, doesn't create new ones" } },
    { pattern: ["test_get_undefined_raises", "test_set_undefined_raises", "test_undefined_in_chain"], message: { zh: "找不到变量时应该抛出 `NameError`", en: "Raise `NameError` when a variable isn't found" } },
  ],
  skeleton: `"""环境模型骨架代码。

你的任务是实现标记为 TODO 的方法。

运行测试：
    pytest learn/03-environment/test_skeleton.py -v
"""

from typing import Dict, Optional, Any


class Environment:
    """环境：存储变量绑定，支持词法作用域。

    环境是一个变量名到值的映射，加上一个指向父环境的引用。
    这种链式结构支持嵌套作用域和变量遮蔽。

    使用方法：
        # 创建全局环境
        global_env = Environment()
        global_env.define('x', 10)

        # 创建子环境（如函数调用时）
        local_env = Environment(parent=global_env)
        local_env.define('y', 20)

        # 查找变量
        local_env.get('x')  # 10（从父环境找到）
        local_env.get('y')  # 20（从当前环境找到）
    """

    def __init__(self, parent: Optional['Environment'] = None):
        """初始化环境。

        Args:
            parent: 父环境，用于实现词法作用域。
                   如果是全局环境，parent 为 None。
        """
        self.bindings: Dict[str, Any] = {}
        self.parent = parent

    def define(self, name: str, value: Any):
        """在当前环境中定义一个新变量。

        TODO: 实现这个方法

        Args:
            name: 变量名
            value: 变量值

        注意：
        - 这个方法总是在当前环境中创建绑定
        - 如果变量已存在，会被覆盖
        - 不会影响父环境

        提示：
        - 直接在 self.bindings 中设置键值对
        """
        # TODO: 实现
        pass

    def get(self, name: str) -> Any:
        """查找变量的值。

        TODO: 实现这个方法

        Args:
            name: 变量名

        Returns:
            变量的值

        Raises:
            NameError: 如果变量未定义

        查找规则：
        1. 先在当前环境的 bindings 中查找
        2. 如果找不到，递归在父环境中查找
        3. 如果到达顶层（parent 为 None）还找不到，抛出 NameError

        提示：
        - 使用 \`in\` 操作符检查键是否存在
        - 使用递归或循环向上查找
        """
        # TODO: 实现
        pass

    def set(self, name: str, value: Any):
        """修改一个已存在变量的值。

        TODO: 实现这个方法

        Args:
            name: 变量名
            value: 新值

        Raises:
            NameError: 如果变量未定义

        注意：
        - 这个方法修改已存在的变量，不创建新变量
        - 需要找到变量所在的环境，然后修改
        - 如果变量在父环境中，应该修改父环境

        提示：
        - 先检查当前环境是否有这个变量
        - 如果有，修改当前环境
        - 如果没有，递归在父环境中查找并修改
        - 如果到达顶层还找不到，抛出 NameError
        """
        # TODO: 实现
        pass

    def __repr__(self):
        """返回环境的字符串表示。"""
        return f"Environment({list(self.bindings.keys())})"

    def __contains__(self, name: str) -> bool:
        """检查变量是否在环境链中定义。

        支持 \`'x' in env\` 语法。
        """
        try:
            self.get(name)
            return True
        except NameError:
            return False`,
  testCode: `"""环境模型测试。

运行测试：
    pytest learn/03-environment/test_skeleton.py -v
"""

import pytest
from skeleton import Environment


class TestBasicOperations:
    """基本操作测试。"""

    def test_define_and_get(self):
        """测试定义和获取变量。"""
        env = Environment()
        env.define('x', 10)

        assert env.get('x') == 10

    def test_define_multiple(self):
        """测试定义多个变量。"""
        env = Environment()
        env.define('x', 10)
        env.define('y', 20)
        env.define('z', 30)

        assert env.get('x') == 10
        assert env.get('y') == 20
        assert env.get('z') == 30

    def test_define_overwrites(self):
        """测试重复定义会覆盖。"""
        env = Environment()
        env.define('x', 10)
        env.define('x', 20)

        assert env.get('x') == 20

    def test_get_undefined_raises(self):
        """测试获取未定义变量抛出错误。"""
        env = Environment()

        with pytest.raises(NameError):
            env.get('x')


class TestParentEnvironment:
    """父环境测试。"""

    def test_get_from_parent(self):
        """测试从父环境获取变量。"""
        parent = Environment()
        parent.define('x', 10)

        child = Environment(parent=parent)

        assert child.get('x') == 10

    def test_child_shadows_parent(self):
        """测试子环境遮蔽父环境变量。"""
        parent = Environment()
        parent.define('x', 10)

        child = Environment(parent=parent)
        child.define('x', 20)

        assert child.get('x') == 20
        assert parent.get('x') == 10  # 父环境不受影响

    def test_deep_nesting(self):
        """测试深层嵌套环境。"""
        env1 = Environment()
        env1.define('a', 1)

        env2 = Environment(parent=env1)
        env2.define('b', 2)

        env3 = Environment(parent=env2)
        env3.define('c', 3)

        # env3 可以访问所有层级的变量
        assert env3.get('a') == 1
        assert env3.get('b') == 2
        assert env3.get('c') == 3

    def test_undefined_in_chain(self):
        """测试在整个环境链中都未定义的变量。"""
        parent = Environment()
        parent.define('x', 10)

        child = Environment(parent=parent)
        child.define('y', 20)

        with pytest.raises(NameError):
            child.get('z')


class TestSetOperation:
    """set 操作测试。"""

    def test_set_local(self):
        """测试修改当前环境的变量。"""
        env = Environment()
        env.define('x', 10)
        env.set('x', 20)

        assert env.get('x') == 20

    def test_set_in_parent(self):
        """测试修改父环境的变量。"""
        parent = Environment()
        parent.define('x', 10)

        child = Environment(parent=parent)
        child.set('x', 20)

        # 父环境的变量被修改
        assert parent.get('x') == 20
        assert child.get('x') == 20

    def test_set_undefined_raises(self):
        """测试修改未定义变量抛出错误。"""
        env = Environment()

        with pytest.raises(NameError):
            env.set('x', 10)

    def test_set_does_not_create(self):
        """测试 set 不会创建新变量。"""
        parent = Environment()

        child = Environment(parent=parent)

        with pytest.raises(NameError):
            child.set('x', 10)


class TestContains:
    """__contains__ 测试。"""

    def test_contains_local(self):
        """测试检查当前环境的变量。"""
        env = Environment()
        env.define('x', 10)

        assert 'x' in env
        assert 'y' not in env

    def test_contains_parent(self):
        """测试检查父环境的变量。"""
        parent = Environment()
        parent.define('x', 10)

        child = Environment(parent=parent)

        assert 'x' in child


class TestRepr:
    """__repr__ 测试。"""

    def test_repr(self):
        """测试字符串表示。"""
        env = Environment()
        env.define('x', 10)
        env.define('y', 20)

        repr_str = repr(env)
        assert 'Environment' in repr_str
        assert 'x' in repr_str or 'y' in repr_str


class TestRealWorldScenarios:
    """真实场景测试。"""

    def test_function_scope(self):
        """模拟函数调用的作用域。"""
        # 全局环境
        global_env = Environment()
        global_env.define('x', 10)

        # 函数调用创建新环境
        func_env = Environment(parent=global_env)
        func_env.define('x', 5)  # 参数 x 遮蔽全局 x
        func_env.define('y', 3)

        # 函数内部
        assert func_env.get('x') == 5  # 使用参数
        assert func_env.get('y') == 3

        # 全局环境不受影响
        assert global_env.get('x') == 10

    def test_nested_function_scope(self):
        """模拟嵌套函数调用。"""
        # 全局
        global_env = Environment()
        global_env.define('a', 1)

        # 外层函数
        outer_env = Environment(parent=global_env)
        outer_env.define('b', 2)

        # 内层函数
        inner_env = Environment(parent=outer_env)
        inner_env.define('c', 3)

        # 内层函数可以访问所有外层变量
        assert inner_env.get('a') == 1
        assert inner_env.get('b') == 2
        assert inner_env.get('c') == 3

    def test_closure_scenario(self):
        """模拟闭包场景。"""
        # 全局环境
        global_env = Environment()

        # make-adder 的环境
        make_adder_env = Environment(parent=global_env)
        make_adder_env.define('x', 5)

        # 返回的闭包捕获 make_adder_env
        # 当闭包被调用时，创建新环境，parent 是 make_adder_env
        closure_call_env = Environment(parent=make_adder_env)
        closure_call_env.define('y', 3)

        # 闭包内部可以访问 x（来自 make_adder_env）和 y（参数）
        assert closure_call_env.get('x') == 5
        assert closure_call_env.get('y') == 3`,
  readme: {
    zh: `# 模块 3：环境模型 (Environment)

> "变量的值存在哪里？"

## 问题引入

考虑这段代码：

\`\`\`lisp
(define x 10)
(define y 20)
(+ x y)  ; 应该得到 30
\`\`\`

当我们执行 \`(+ x y)\` 时，需要知道 \`x\` 和 \`y\` 的值。

**问题**：这些值存在哪里？如何查找？

---

## 更复杂的情况

\`\`\`lisp
(define x 10)

(define f
  (lambda (x)
    (+ x 1)))

(f 5)  ; 结果是什么？6 还是 11？
\`\`\`

这里有两个 \`x\`：
- 外部的 \`x = 10\`
- 函数参数 \`x = 5\`

**问题**：函数内部的 \`x\` 应该是哪个？

答案：**5**。函数参数"遮蔽"了外部变量。

---

## 核心概念

### 环境是什么？

环境是一个**变量名到值的映射**，加上一个**指向父环境的引用**。

\`\`\`
┌─────────────────────┐
│ 全局环境            │
│ x → 10              │
│ y → 20              │
│ f → <closure>       │
└─────────────────────┘
         ↑
         │ parent
┌─────────────────────┐
│ 函数环境            │
│ x → 5               │  ← 遮蔽了全局的 x
└─────────────────────┘
\`\`\`

### 变量查找规则

1. 在当前环境中查找
2. 如果找不到，在父环境中查找
3. 一直向上，直到找到或到达顶层
4. 如果顶层也没有，报错"未定义变量"

这就是**词法作用域**（Lexical Scoping）。

### 三个核心操作

| 操作 | 说明 | 示例 |
|------|------|------|
| \`define(name, value)\` | 在当前环境定义变量 | \`(define x 10)\` |
| \`get(name)\` | 查找变量值 | 求值 \`x\` |
| \`set(name, value)\` | 修改已存在的变量 | \`(set! x 20)\` |

---

## 关键不变量

1. **词法作用域**：变量查找遵循代码的静态结构，而不是运行时调用栈
2. **遮蔽规则**：内层环境的变量遮蔽外层同名变量
3. **define vs set**：\`define\` 在当前环境创建，\`set\` 修改已存在的变量

---

## 动手实现

### 步骤 1：理解骨架代码

打开 \`skeleton.py\`，你会看到：

\`\`\`python
class Environment:
    def __init__(self, parent=None):
        self.bindings = {}
        self.parent = parent

    def define(self, name, value):
        # TODO
        pass

    def get(self, name):
        # TODO
        pass
\`\`\`

### 步骤 2：实现核心方法

你需要实现：

1. \`define(name, value)\` - 在当前环境定义变量
2. \`get(name)\` - 查找变量值（需要向上查找父环境）
3. \`set(name, value)\` - 修改已存在的变量

### 步骤 3：运行测试

\`\`\`bash
cd tiny-interpreter
pytest learn/03-environment/test_skeleton.py -v
\`\`\`

---

## 可视化

让我们看看环境链是如何工作的：

\`\`\`lisp
(define x 10)
(define f (lambda (y) (+ x y)))
(f 5)
\`\`\`

执行过程：

\`\`\`
1. 创建全局环境
   ┌─────────────┐
   │ x → 10      │
   │ f → closure │
   └─────────────┘

2. 调用 (f 5)，创建函数环境
   ┌─────────────┐
   │ x → 10      │
   │ f → closure │
   └─────────────┘
         ↑
   ┌─────────────┐
   │ y → 5       │  ← 当前环境
   └─────────────┘

3. 求值 (+ x y)
   - 查找 x：当前环境没有 → 父环境找到 x=10
   - 查找 y：当前环境找到 y=5
   - 结果：15
\`\`\`

---

## 提示

如果卡住了，可以查看提示：

- [提示 1：define 和 get](hints/hint1.md)
- [提示 2：向上查找](hints/hint2.md)
- [提示 3：set 的实现](hints/hint3.md)

---

## 深入思考

完成实现后，思考这些问题（详见 [challenge.md](challenge.md)）：

1. 为什么需要 \`parent\` 引用？能不能用其他方式？
2. \`set\` 和 \`define\` 的区别是什么？
3. 如果允许删除变量，会有什么问题？

---

## 下一步

现在我们有了存储变量的地方。

接下来，让我们实现求值器，把 AST 和环境结合起来！

[进入模块 4：基础求值 →](../04-evaluator-basic/README.md)`,
    en: `# Module 3: Environment Model

> "Where are variable values stored?"

## Problem Introduction

Consider this code:

\`\`\`lisp
(define x 10)
(define y 20)
(+ x y)  ; should return 30
\`\`\`

When we execute \`(+ x y)\`, we need to know the values of \`x\` and \`y\`.

**Question**: Where are these values stored? How do we look them up?

---

## A More Complex Case

\`\`\`lisp
(define x 10)

(define f
  (lambda (x)
    (+ x 1)))

(f 5)  ; What's the result? 6 or 11?
\`\`\`

There are two \`x\`s here:
- The outer \`x = 10\`
- The function parameter \`x = 5\`

**Question**: Which \`x\` should be used inside the function?

Answer: **5**. The function parameter "shadows" the outer variable.

---

## Core Concepts

### What is an Environment?

An environment is a **mapping from variable names to values**, plus a **reference to a parent environment**.

\`\`\`
┌─────────────────────┐
│ Global Environment  │
│ x → 10              │
│ y → 20              │
│ f → <closure>       │
└─────────────────────┘
         ↑
         │ parent
┌─────────────────────┐
│ Function Environment│
│ x → 5               │  ← shadows global x
└─────────────────────┘
\`\`\`

### Variable Lookup Rules

1. Look up in the current environment
2. If not found, look up in the parent environment
3. Keep going up until found or reaching the top level
4. If not found at the top level, raise "undefined variable" error

This is **Lexical Scoping**.

### Three Core Operations

| Operation | Description | Example |
|-----------|-------------|---------|
| \`define(name, value)\` | Define a variable in the current environment | \`(define x 10)\` |
| \`get(name)\` | Look up a variable's value | Evaluating \`x\` |
| \`set(name, value)\` | Modify an existing variable | \`(set! x 20)\` |

---

## Key Invariants

1. **Lexical scoping**: Variable lookup follows the static structure of the code, not the runtime call stack
2. **Shadowing rule**: Variables in inner environments shadow same-named variables in outer environments
3. **define vs set**: \`define\` creates in the current environment, \`set\` modifies an existing variable

---

## Hands-on Implementation

### Step 1: Understand the Skeleton Code

Open \`skeleton.py\`, you'll see:

\`\`\`python
class Environment:
    def __init__(self, parent=None):
        self.bindings = {}
        self.parent = parent

    def define(self, name, value):
        # TODO
        pass

    def get(self, name):
        # TODO
        pass
\`\`\`

### Step 2: Implement Core Methods

You need to implement:

1. \`define(name, value)\` - Define a variable in the current environment
2. \`get(name)\` - Look up a variable's value (needs to search parent environments)
3. \`set(name, value)\` - Modify an existing variable

### Step 3: Run Tests

\`\`\`bash
cd tiny-interpreter
pytest learn/03-environment/test_skeleton.py -v
\`\`\`

---

## Visualization

Let's see how the environment chain works:

\`\`\`lisp
(define x 10)
(define f (lambda (y) (+ x y)))
(f 5)
\`\`\`

Execution process:

\`\`\`
1. Create global environment
   ┌─────────────┐
   │ x → 10      │
   │ f → closure │
   └─────────────┘

2. Call (f 5), create function environment
   ┌─────────────┐
   │ x → 10      │
   │ f → closure │
   └─────────────┘
         ↑
   ┌─────────────┐
   │ y → 5       │  ← current environment
   └─────────────┘

3. Evaluate (+ x y)
   - Look up x: not in current env → found in parent env x=10
   - Look up y: found in current env y=5
   - Result: 15
\`\`\`

---

## Hints

If you get stuck, check the hints:

- [Hint 1: define and get](hints/hint1.md)
- [Hint 2: Upward lookup](hints/hint2.md)
- [Hint 3: set implementation](hints/hint3.md)

---

## Deep Thinking

After completing the implementation, think about these questions (see [challenge.md](challenge.md)):

1. Why do we need the \`parent\` reference? Could we use another approach?
2. What's the difference between \`set\` and \`define\`?
3. What problems would arise if we allowed deleting variables?

---

## Next Step

Now we have a place to store variables.

Next, let's implement the evaluator to combine AST and environment!

[Enter Module 4: Basic Evaluation →](../04-evaluator-basic/README.md)`,
  },
  hints: [
    { level: 1, content: { zh: `# 提示 1：define 和基本的 get

## define() 实现

\`define\` 很简单——直接在当前环境的 bindings 中设置：

\`\`\`python
def define(self, name: str, value: Any):
    self.bindings[name] = value
\`\`\`

就这么简单！\`define\` 总是在当前环境创建绑定。

## 基本的 get() 实现（不考虑父环境）

先实现一个只在当前环境查找的版本：

\`\`\`python
def get(self, name: str) -> Any:
    if name in self.bindings:
        return self.bindings[name]
    raise NameError(f"Undefined variable: {name}")
\`\`\`

## 下一步

这个版本的 \`get\` 还不能从父环境查找。

查看 [hint2.md](hint2.md) 了解如何实现向上查找。`, en: `# Hint 1: define and Basic get

## define() Implementation

\`define\` is simple — just set the value in the current environment's bindings:

\`\`\`python
def define(self, name: str, value: Any):
    self.bindings[name] = value
\`\`\`

That's it! \`define\` always creates a binding in the current environment.

## Basic get() Implementation (Without Parent Lookup)

First, implement a version that only looks in the current environment:

\`\`\`python
def get(self, name: str) -> Any:
    if name in self.bindings:
        return self.bindings[name]
    raise NameError(f"Undefined variable: {name}")
\`\`\`

## Next Step

This version of \`get\` can't look up variables in parent environments yet.

Check [hint2.md](hint2.md) to learn how to implement upward lookup.` } },
    { level: 2, content: { zh: `# 提示 2：向上查找

## get() 的完整实现

关键是：如果当前环境没有，就去父环境找。

\`\`\`python
def get(self, name: str) -> Any:
    # 1. 先在当前环境查找
    if name in self.bindings:
        return self.bindings[name]

    # 2. 如果有父环境，递归查找
    if self.parent is not None:
        return self.parent.get(name)

    # 3. 到达顶层还没找到，报错
    raise NameError(f"Undefined variable: {name}")
\`\`\`

## 递归 vs 循环

上面用的是递归。也可以用循环：

\`\`\`python
def get(self, name: str) -> Any:
    env = self
    while env is not None:
        if name in env.bindings:
            return env.bindings[name]
        env = env.parent
    raise NameError(f"Undefined variable: {name}")
\`\`\`

两种方式都可以，递归更简洁，循环更高效（避免栈溢出）。

## 下一步

现在尝试实现 \`set()\`。

查看 [hint3.md](hint3.md) 了解 \`set\` 的实现。`, en: `# Hint 2: Upward Lookup

## Complete get() Implementation

The key is: if the current environment doesn't have it, look in the parent.

\`\`\`python
def get(self, name: str) -> Any:
    # 1. First look in the current environment
    if name in self.bindings:
        return self.bindings[name]

    # 2. If there's a parent environment, search recursively
    if self.parent is not None:
        return self.parent.get(name)

    # 3. Reached the top level without finding it, raise error
    raise NameError(f"Undefined variable: {name}")
\`\`\`

## Recursive vs Iterative

The above uses recursion. You can also use a loop:

\`\`\`python
def get(self, name: str) -> Any:
    env = self
    while env is not None:
        if name in env.bindings:
            return env.bindings[name]
        env = env.parent
    raise NameError(f"Undefined variable: {name}")
\`\`\`

Both approaches work. Recursion is more concise; iteration is more efficient (avoids stack overflow).

## Next Step

Now try implementing \`set()\`.

Check [hint3.md](hint3.md) for the \`set\` implementation.` } },
    { level: 3, content: { zh: `# 提示 3：set 的实现

## set() vs define()

关键区别：
- \`define\`：总是在**当前**环境创建绑定
- \`set\`：修改**已存在**的变量，可能在父环境中

## set() 的完整实现

\`\`\`python
def set(self, name: str, value: Any):
    # 1. 如果当前环境有这个变量，修改它
    if name in self.bindings:
        self.bindings[name] = value
        return

    # 2. 如果有父环境，递归查找并修改
    if self.parent is not None:
        self.parent.set(name, value)
        return

    # 3. 到达顶层还没找到，报错
    raise NameError(f"Undefined variable: {name}")
\`\`\`

## 为什么 set 不创建新变量？

考虑这个例子：

\`\`\`lisp
(define x 10)

(define f
  (lambda ()
    (set! x 20)))  ; 应该修改全局的 x

(f)
x  ; 应该是 20
\`\`\`

如果 \`set\` 在当前环境创建新变量，就会遮蔽全局的 \`x\`，而不是修改它。

## 完整实现

如果你还是卡住了，可以参考 \`src/tiny_interpreter/environment.py\` 中的完整实现。`, en: `# Hint 3: set Implementation

## set() vs define()

Key difference:
- \`define\`: always creates a binding in the **current** environment
- \`set\`: modifies an **existing** variable, possibly in a parent environment

## Complete set() Implementation

\`\`\`python
def set(self, name: str, value: Any):
    # 1. If the current environment has this variable, modify it
    if name in self.bindings:
        self.bindings[name] = value
        return

    # 2. If there's a parent environment, search and modify recursively
    if self.parent is not None:
        self.parent.set(name, value)
        return

    # 3. Reached the top level without finding it, raise error
    raise NameError(f"Undefined variable: {name}")
\`\`\`

## Why Doesn't set Create New Variables?

Consider this example:

\`\`\`lisp
(define x 10)

(define f
  (lambda ()
    (set! x 20)))  ; should modify the global x

(f)
x  ; should be 20
\`\`\`

If \`set\` created a new variable in the current environment, it would shadow the global \`x\` instead of modifying it.

## Complete Implementation

If you're still stuck, you can refer to the complete implementation in \`src/tiny_interpreter/environment.py\`.` } },
  ],
  guidance: [
    {
      question: {
        zh: "最简单的 `define()` 应该做什么？它需要几行代码？",
        en: "What's the simplest thing `define()` should do? How many lines of code?"
      }
    },
    {
      question: {
        zh: "`get()` 在当前环境找不到变量时应该怎么办？",
        en: "What should `get()` do when it can't find a variable in the current environment?"
      }
    },
    {
      question: {
        zh: "环境链是如何实现词法作用域的？`parent` 引用指向哪里？",
        en: "How does the environment chain implement lexical scoping? Where does the `parent` reference point?"
      }
    },
    {
      question: {
        zh: "`set()` 和 `define()` 的关键区别是什么？",
        en: "What's the key difference between `set()` and `define()`?"
      }
    },
    {
      question: {
        zh: "如果变量在父环境中，`set()` 应该修改哪个环境？",
        en: "If a variable is in the parent environment, which environment should `set()` modify?"
      }
    },
    {
      question: {
        zh: "为什么 `set()` 不能创建新变量？这个设计有什么好处？",
        en: "Why can't `set()` create new variables? What's the benefit of this design?"
      }
    }
  ],
};