# 模块 1：词法分析 (Lexer)

> "如何把字符串变成有意义的单元？"

## 问题引入

假设你收到一个字符串 `"(+ 1 2)"`，你需要理解它的含义。

**第一步**：把字符串分解成有意义的"单词"。

```
"(+ 1 2)"  →  ["(", "+", "1", "2", ")"]
```

这些"单词"在编译原理中叫做 **Token**（词法单元）。

**思考**：
- 空格去哪了？
- 如何区分 `+`（运算符）和 `123`（数字）？
- 如何处理 `-42`（负数）和 `- 42`（减法）？

---

## 核心概念

### Token 是什么？

Token 是源代码的最小有意义单元。每个 Token 包含：

| 属性 | 说明 | 示例 |
|------|------|------|
| type | Token 类型 | NUMBER, SYMBOL, LPAREN |
| value | Token 值 | 42, "+", "(" |
| line | 行号 | 1 |
| column | 列号 | 5 |

### 我们的 Token 类型

```
LPAREN   →  (
RPAREN   →  )
NUMBER   →  42, -10
SYMBOL   →  +, define, foo-bar
BOOLEAN  →  #t, #f
EOF      →  输入结束
```

### Lexer 的工作流程

```
输入: "(+ 1 2)"

位置:  0 1 2 3 4 5 6
字符:  ( +   1   2 )

步骤:
  pos=0: 看到 '(' → 生成 LPAREN
  pos=1: 看到 '+' → 生成 SYMBOL("+")
  pos=2: 看到 ' ' → 跳过空格
  pos=3: 看到 '1' → 生成 NUMBER(1)
  pos=4: 看到 ' ' → 跳过空格
  pos=5: 看到 '2' → 生成 NUMBER(2)
  pos=6: 看到 ')' → 生成 RPAREN
  pos=7: 到达末尾 → 生成 EOF

输出: [LPAREN, SYMBOL("+"), NUMBER(1), NUMBER(2), RPAREN, EOF]
```

---

## 关键不变量

实现 Lexer 时，必须保证：

1. **完整性**：每个字符都被处理（要么生成 Token，要么被跳过）
2. **无歧义**：每个字符序列只能解析成一种 Token
3. **位置准确**：Token 的行号和列号必须正确
4. **错误处理**：遇到非法字符时抛出有意义的错误

---

## 动手实现

### 步骤 1：理解骨架代码

打开 `skeleton.py`，你会看到：

```python
class Lexer:
    def __init__(self, source: str):
        self.source = source
        self.pos = 0
        self.line = 1
        self.column = 1

    def next_token(self) -> Token:
        # TODO: 实现这个方法
        pass
```

### 步骤 2：实现核心方法

你需要实现以下方法：

1. `current_char()` - 返回当前字符
2. `advance()` - 前进一个字符
3. `skip_whitespace()` - 跳过空白字符
4. `read_number()` - 读取数字
5. `read_symbol()` - 读取符号
6. `next_token()` - 返回下一个 Token

### 步骤 3：运行测试

```bash
cd tiny-interpreter
pytest learn/01-lexer/test_skeleton.py -v
```

测试是渐进式的，从简单到复杂：
- `test_empty_input` - 空输入
- `test_single_number` - 单个数字
- `test_parentheses` - 括号
- `test_simple_expression` - 简单表达式
- ...

---

## 提示

如果卡住了，可以查看提示：

- [提示 1：基本思路](hints/hint1.md)
- [提示 2：处理数字](hints/hint2.md)
- [提示 3：完整实现思路](hints/hint3.md)

---

## 验证成功

当所有测试通过时，你已经完成了词法分析器！

```bash
$ pytest learn/01-lexer/test_skeleton.py -v
...
test_empty_input PASSED
test_single_number PASSED
test_simple_expression PASSED
...
```

---

## 深入思考

完成实现后，思考这些问题：

1. 为什么要分开 `current_char()` 和 `advance()`？
2. 如何支持浮点数？需要修改哪些地方？
3. 如何支持字符串字面量（如 `"hello"`）？

这些问题在 [challenge.md](challenge.md) 中有更详细的讨论。

---

## 下一步

完成词法分析后，我们有了 Token 序列。

但 Token 序列是"扁平"的，我们需要把它组织成"树状"结构。

[进入模块 2：语法分析 →](../02-parser/README.md)
