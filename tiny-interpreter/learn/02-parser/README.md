# 模块 2：语法分析 (Parser)

> "如何把 Token 组织成树状结构？"

## 问题引入

词法分析给了我们一个 Token 序列：

```
[LPAREN, SYMBOL("+"), NUMBER(1), LPAREN, SYMBOL("*"), NUMBER(2), NUMBER(3), RPAREN, RPAREN]
```

这是"扁平"的。但表达式 `(+ 1 (* 2 3))` 有层次结构：

```
    +
   / \
  1   *
     / \
    2   3
```

**问题**：如何从扁平的 Token 序列构建这棵树？

---

## 核心概念

### 抽象语法树 (AST)

AST 是程序的树状表示。每个节点代表一个语法结构：

| 节点类型 | 说明 | 示例 |
|---------|------|------|
| Number | 数字字面量 | `42` |
| Boolean | 布尔字面量 | `#t` |
| Symbol | 符号 | `+`, `define` |
| SExpression | S-表达式（列表） | `(+ 1 2)` |

### S-表达式

Lisp 的语法非常简单：一切都是 S-表达式。

```
S-表达式 = 原子 | (S-表达式*)

原子 = 数字 | 布尔值 | 符号
```

这意味着：
- `42` 是 S-表达式（原子）
- `(+ 1 2)` 是 S-表达式（列表）
- `(+ 1 (* 2 3))` 是 S-表达式（嵌套列表）

### 递归下降解析

我们使用**递归下降**方法解析：

```
parse_expr():
    if 当前是 '(':
        return parse_sexp()  # 解析列表
    else:
        return parse_atom()  # 解析原子

parse_sexp():
    expect '('
    elements = []
    while 当前不是 ')':
        elements.append(parse_expr())  # 递归！
    expect ')'
    return SExpression(elements)
```

关键洞察：`parse_expr()` 调用 `parse_sexp()`，而 `parse_sexp()` 又调用 `parse_expr()`。这种**相互递归**自然地处理了嵌套结构。

---

## 关键不变量

1. **括号匹配**：每个 `(` 必须有对应的 `)`
2. **完整消费**：解析完成后，所有 Token 都被处理（除了 EOF）
3. **结构正确**：AST 的结构反映源代码的嵌套关系

---

## 动手实现

### 步骤 1：理解骨架代码

打开 `skeleton.py`，你会看到：

```python
class Parser:
    def __init__(self, tokens: List[Token]):
        self.tokens = tokens
        self.pos = 0

    def parse_expr(self) -> ASTNode:
        # TODO: 实现
        pass
```

### 步骤 2：实现核心方法

你需要实现：

1. `current_token()` - 返回当前 Token
2. `advance()` - 前进到下一个 Token
3. `expect(token_type)` - 期望特定类型的 Token
4. `parse_atom()` - 解析原子（数字、布尔值、符号）
5. `parse_sexp()` - 解析 S-表达式
6. `parse_expr()` - 解析表达式（原子或 S-表达式）

### 步骤 3：运行测试

```bash
cd tiny-interpreter
pytest learn/02-parser/test_skeleton.py -v
```

---

## 可视化

让我们看看 `(+ 1 (* 2 3))` 是如何被解析的：

```
Token 序列: ( + 1 ( * 2 3 ) )
            0 1 2 3 4 5 6 7 8

parse_expr() at pos=0
  看到 '(' → 调用 parse_sexp()
    expect '(' ✓, pos=1
    parse_expr() at pos=1
      看到 '+' → 调用 parse_atom()
        返回 Symbol('+'), pos=2
    parse_expr() at pos=2
      看到 '1' → 调用 parse_atom()
        返回 Number(1), pos=3
    parse_expr() at pos=3
      看到 '(' → 调用 parse_sexp()
        expect '(' ✓, pos=4
        parse_expr() at pos=4
          返回 Symbol('*'), pos=5
        parse_expr() at pos=5
          返回 Number(2), pos=6
        parse_expr() at pos=6
          返回 Number(3), pos=7
        expect ')' ✓, pos=8
        返回 SExpression([Symbol('*'), Number(2), Number(3)])
    expect ')' ✓, pos=9
    返回 SExpression([Symbol('+'), Number(1), SExpression(...)])
```

---

## 提示

如果卡住了，可以查看提示：

- [提示 1：基本结构](hints/hint1.md)
- [提示 2：递归解析](hints/hint2.md)
- [提示 3：完整实现](hints/hint3.md)

---

## 深入思考

完成实现后，思考这些问题（详见 [challenge.md](challenge.md)）：

1. 如果括号不匹配会发生什么？
2. 如何提供更好的错误信息？
3. 这种解析方法的局限性是什么？

---

## 下一步

现在我们有了 AST，但还不能执行它。

执行需要知道变量的值存在哪里。

[进入模块 3：环境模型 →](../03-environment/README.md)
