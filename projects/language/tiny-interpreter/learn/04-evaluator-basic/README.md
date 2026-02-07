# 模块 4：基础求值 (Evaluator - Basic)

> "如何计算表达式的值？"

## 问题引入

我们有了：
- AST（程序的树状表示）
- Environment（存储变量的地方）

现在需要把它们结合起来，**执行**程序。

```lisp
(+ 1 (* 2 3))
```

**问题**：如何计算这个表达式的值？

---

## 核心概念

### 求值规则

不同类型的 AST 节点有不同的求值规则：

| 节点类型 | 求值规则 | 示例 |
|---------|---------|------|
| Number | 返回自身 | `42` → `42` |
| Boolean | 返回自身 | `#t` → `True` |
| Symbol | 在环境中查找 | `x` → `env.get('x')` |
| SExpression | 特殊形式或函数调用 | `(+ 1 2)` → `3` |

### 自求值表达式

数字和布尔值是"自求值"的——它们的值就是自己：

```python
def eval(node, env):
    if isinstance(node, Number):
        return node.value  # 42 → 42

    if isinstance(node, Boolean):
        return node.value  # #t → True
```

### 变量查找

符号需要在环境中查找：

```python
    if isinstance(node, Symbol):
        return env.get(node.name)  # x → env.get('x')
```

### 函数调用

S-表达式通常是函数调用：

```lisp
(+ 1 2)
```

求值步骤：
1. 求值第一个元素，得到函数 `+`
2. 求值其余元素，得到参数 `[1, 2]`
3. 调用函数：`+(1, 2)` → `3`

---

## 特殊形式

有些 S-表达式不是普通的函数调用，而是**特殊形式**：

| 特殊形式 | 说明 | 示例 |
|---------|------|------|
| `define` | 定义变量 | `(define x 10)` |
| `if` | 条件表达式 | `(if #t 1 2)` |
| `quote` | 阻止求值 | `(quote (1 2 3))` |
| `begin` | 顺序执行 | `(begin e1 e2 e3)` |

特殊形式的参数**不会**自动求值，需要特殊处理。

### 为什么 if 是特殊形式？

考虑：

```lisp
(if #t 1 (/ 1 0))
```

如果 `if` 是普通函数，所有参数都会先求值，`(/ 1 0)` 会报错。

但 `if` 是特殊形式，只有条件为真时才求值 then 分支，条件为假时才求值 else 分支。

---

## 动手实现

### 步骤 1：理解骨架代码

打开 `skeleton.py`，你会看到：

```python
class Evaluator:
    def eval(self, node, env):
        # TODO: 实现
        pass
```

### 步骤 2：实现核心方法

你需要实现：

1. `eval(node, env)` - 主求值函数
2. `eval_define(args, env)` - 处理 define
3. `eval_if(args, env)` - 处理 if
4. `eval_application(elements, env)` - 处理函数调用

### 步骤 3：运行测试

```bash
cd tiny-interpreter
pytest learn/04-evaluator-basic/test_skeleton.py -v
```

---

## 可视化

让我们看看 `(+ 1 (* 2 3))` 是如何求值的：

```
eval((+ 1 (* 2 3)), env)
  → 这是 SExpression，不是特殊形式
  → 函数调用！

  1. 求值函数：eval(+, env) → <builtin +>
  2. 求值参数：
     - eval(1, env) → 1
     - eval((* 2 3), env)
         → 函数调用！
         → 求值函数：eval(*, env) → <builtin *>
         → 求值参数：eval(2, env) → 2, eval(3, env) → 3
         → 调用：*(2, 3) → 6
  3. 调用：+(1, 6) → 7

结果：7
```

---

## 提示

如果卡住了，可以查看提示：

- [提示 1：自求值和变量查找](hints/hint1.md)
- [提示 2：特殊形式](hints/hint2.md)
- [提示 3：函数调用](hints/hint3.md)

---

## 深入思考

完成实现后，思考这些问题（详见 [challenge.md](challenge.md)）：

1. 为什么 `define` 不返回值？
2. `if` 的两个分支必须都有吗？
3. 如何实现 `and` 和 `or`？

---

## 下一步

现在我们可以执行基本的表达式了。

但还缺少最重要的部分：**函数定义**（lambda）和**闭包**。

这是整个项目的"顿悟时刻"！

[进入模块 5：Lambda 与闭包 →](../05-evaluator-lambda/README.md)
