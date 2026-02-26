import type { LearningModule } from "../types";

export const module01Lexer: LearningModule = {
  id: "01-lexer",
  index: 1,
  slug: "lexer",
  hasCode: true,
  vizType: "tokens",
  skeleton: `"""词法分析器骨架代码。

你的任务是实现标记为 TODO 的方法。

运行测试：
    pytest learn/01-lexer/test_skeleton.py -v
"""

from dataclasses import dataclass
from enum import Enum, auto
from typing import List, Optional


class TokenType(Enum):
    """Token 类型。"""
    LPAREN = auto()      # (
    RPAREN = auto()      # )
    NUMBER = auto()      # 123, -42
    SYMBOL = auto()      # foo, +, define
    BOOLEAN = auto()     # #t, #f
    EOF = auto()         # 输入结束


@dataclass
class Token:
    """一个 Token，包含类型、值和位置信息。"""
    type: TokenType
    value: any
    line: int
    column: int

    def __repr__(self):
        return f"Token({self.type.name}, {self.value!r}, {self.line}:{self.column})"


class LexerError(Exception):
    """词法分析错误。"""
    def __init__(self, message: str, line: int, column: int):
        self.message = message
        self.line = line
        self.column = column
        super().__init__(f"{message} at line {line}, column {column}")


class Lexer:
    """词法分析器：将源代码字符串转换为 Token 序列。

    使用方法：
        lexer = Lexer("(+ 1 2)")
        tokens = lexer.tokenize()
    """

    def __init__(self, source: str):
        """初始化 Lexer。

        Args:
            source: 源代码字符串
        """
        self.source = source
        self.pos = 0        # 当前位置
        self.line = 1       # 当前行号（从 1 开始）
        self.column = 1     # 当前列号（从 1 开始）

    def current_char(self) -> Optional[str]:
        """返回当前位置的字符，如果到达末尾返回 None。

        TODO: 实现这个方法

        提示：
        - 检查 self.pos 是否超出 self.source 的长度
        - 如果超出，返回 None
        - 否则返回 self.source[self.pos]
        """
        # TODO: 实现
        pass

    def peek_char(self, offset: int = 1) -> Optional[str]:
        """查看后面的字符，但不移动位置。

        Args:
            offset: 向前看的偏移量，默认为 1

        Returns:
            偏移位置的字符，如果超出范围返回 None
        """
        pos = self.pos + offset
        if pos >= len(self.source):
            return None
        return self.source[pos]

    def advance(self) -> Optional[str]:
        """消费当前字符并前进到下一个位置。

        TODO: 实现这个方法

        提示：
        - 先获取当前字符
        - 如果不是 None，更新位置和行列号
        - 如果字符是换行符 '\\n'，行号加 1，列号重置为 1
        - 否则列号加 1
        - 返回消费的字符
        """
        # TODO: 实现
        pass

    def skip_whitespace(self):
        """跳过空白字符（空格、制表符、换行符）。

        TODO: 实现这个方法

        提示：
        - 使用 while 循环
        - 检查当前字符是否是空白字符（使用 str.isspace()）
        - 如果是，调用 advance() 跳过
        """
        # TODO: 实现
        pass

    def skip_comment(self):
        """跳过注释（从 ; 到行尾）。

        TODO: 实现这个方法

        提示：
        - 检查当前字符是否是 ';'
        - 如果是，一直 advance() 直到遇到换行符或文件结束
        """
        # TODO: 实现
        pass

    def read_number(self) -> Token:
        """读取一个数字 Token。

        TODO: 实现这个方法

        提示：
        - 记录起始位置（用于 Token 的 line 和 column）
        - 处理可能的负号
        - 收集所有数字字符
        - 转换为整数
        - 返回 NUMBER 类型的 Token
        """
        # TODO: 实现
        pass

    def read_symbol(self) -> Token:
        """读取一个符号 Token。

        TODO: 实现这个方法

        提示：
        - 记录起始位置
        - 收集所有合法的符号字符（使用 is_symbol_char）
        - 返回 SYMBOL 类型的 Token
        """
        # TODO: 实现
        pass

    def read_boolean(self) -> Token:
        """读取一个布尔值 Token (#t 或 #f)。

        TODO: 实现这个方法

        提示：
        - 记录起始位置
        - 跳过 '#'
        - 检查下一个字符是 't' 还是 'f'
        - 如果都不是，抛出 LexerError
        """
        # TODO: 实现
        pass

    def is_symbol_char(self, char: str) -> bool:
        """检查字符是否可以作为符号的一部分。

        符号可以包含：字母、数字、以及 +-*/=<>!?_

        Args:
            char: 要检查的字符

        Returns:
            如果字符可以作为符号的一部分，返回 True
        """
        return (char.isalnum() or char in '+-*/=<>!?_')

    def next_token(self) -> Token:
        """读取并返回下一个 Token。

        TODO: 实现这个方法

        这是 Lexer 的核心方法。它应该：
        1. 跳过空白字符
        2. 跳过注释
        3. 检查是否到达文件末尾（返回 EOF Token）
        4. 根据当前字符决定读取什么类型的 Token：
           - '(' → LPAREN
           - ')' → RPAREN
           - '#' → 布尔值
           - 数字或负号后跟数字 → NUMBER
           - 其他符号字符 → SYMBOL
        5. 如果遇到无法识别的字符，抛出 LexerError
        """
        # TODO: 实现
        pass

    def tokenize(self) -> List[Token]:
        """将整个源代码转换为 Token 列表。

        Returns:
            Token 列表，以 EOF Token 结尾
        """
        tokens = []
        while True:
            token = self.next_token()
            tokens.append(token)
            if token.type == TokenType.EOF:
                break
        return tokens`,
  testCode: `"""词法分析器测试。

这些测试是渐进式的，从简单到复杂。
建议按顺序通过每个测试。

运行测试：
    pytest learn/01-lexer/test_skeleton.py -v
"""

import pytest
from skeleton import Lexer, TokenType, LexerError


class TestBasics:
    """基础测试：空输入和简单 Token。"""

    def test_empty_input(self):
        """测试空输入应该只返回 EOF。"""
        lexer = Lexer("")
        tokens = lexer.tokenize()

        assert len(tokens) == 1
        assert tokens[0].type == TokenType.EOF

    def test_whitespace_only(self):
        """测试只有空白字符的输入。"""
        lexer = Lexer("   \\n\\t  ")
        tokens = lexer.tokenize()

        assert len(tokens) == 1
        assert tokens[0].type == TokenType.EOF


class TestNumbers:
    """数字测试。"""

    def test_single_number(self):
        """测试单个数字。"""
        lexer = Lexer("42")
        tokens = lexer.tokenize()

        assert len(tokens) == 2  # NUMBER + EOF
        assert tokens[0].type == TokenType.NUMBER
        assert tokens[0].value == 42

    def test_negative_number(self):
        """测试负数。"""
        lexer = Lexer("-42")
        tokens = lexer.tokenize()

        assert len(tokens) == 2
        assert tokens[0].type == TokenType.NUMBER
        assert tokens[0].value == -42

    def test_multiple_numbers(self):
        """测试多个数字。"""
        lexer = Lexer("1 2 3")
        tokens = lexer.tokenize()

        assert len(tokens) == 4  # 3 numbers + EOF
        assert all(t.type == TokenType.NUMBER for t in tokens[:-1])
        assert [t.value for t in tokens[:-1]] == [1, 2, 3]


class TestSymbols:
    """符号测试。"""

    def test_single_symbol(self):
        """测试单个符号。"""
        lexer = Lexer("foo")
        tokens = lexer.tokenize()

        assert len(tokens) == 2
        assert tokens[0].type == TokenType.SYMBOL
        assert tokens[0].value == "foo"

    def test_operator_symbols(self):
        """测试运算符符号。"""
        lexer = Lexer("+ - * /")
        tokens = lexer.tokenize()

        assert len(tokens) == 5  # 4 symbols + EOF
        assert [t.value for t in tokens[:-1]] == ["+", "-", "*", "/"]

    def test_symbol_with_special_chars(self):
        """测试带特殊字符的符号。"""
        lexer = Lexer("foo-bar? baz!")
        tokens = lexer.tokenize()

        assert tokens[0].value == "foo-bar?"
        assert tokens[1].value == "baz!"


class TestBooleans:
    """布尔值测试。"""

    def test_boolean_true(self):
        """测试 #t。"""
        lexer = Lexer("#t")
        tokens = lexer.tokenize()

        assert len(tokens) == 2
        assert tokens[0].type == TokenType.BOOLEAN
        assert tokens[0].value is True

    def test_boolean_false(self):
        """测试 #f。"""
        lexer = Lexer("#f")
        tokens = lexer.tokenize()

        assert len(tokens) == 2
        assert tokens[0].type == TokenType.BOOLEAN
        assert tokens[0].value is False

    def test_invalid_boolean(self):
        """测试非法布尔值应该抛出错误。"""
        lexer = Lexer("#x")
        with pytest.raises(LexerError):
            lexer.tokenize()


class TestParentheses:
    """括号测试。"""

    def test_parentheses(self):
        """测试括号。"""
        lexer = Lexer("()")
        tokens = lexer.tokenize()

        assert len(tokens) == 3  # LPAREN + RPAREN + EOF
        assert tokens[0].type == TokenType.LPAREN
        assert tokens[1].type == TokenType.RPAREN


class TestExpressions:
    """表达式测试。"""

    def test_simple_expression(self):
        """测试简单表达式 (+ 1 2)。"""
        lexer = Lexer("(+ 1 2)")
        tokens = lexer.tokenize()

        assert len(tokens) == 6  # ( + 1 2 ) EOF
        assert tokens[0].type == TokenType.LPAREN
        assert tokens[1].type == TokenType.SYMBOL
        assert tokens[1].value == "+"
        assert tokens[2].type == TokenType.NUMBER
        assert tokens[2].value == 1
        assert tokens[3].type == TokenType.NUMBER
        assert tokens[3].value == 2
        assert tokens[4].type == TokenType.RPAREN

    def test_nested_expression(self):
        """测试嵌套表达式 (+ (* 2 3) 4)。"""
        lexer = Lexer("(+ (* 2 3) 4)")
        tokens = lexer.tokenize()

        expected_types = [
            TokenType.LPAREN,   # (
            TokenType.SYMBOL,   # +
            TokenType.LPAREN,   # (
            TokenType.SYMBOL,   # *
            TokenType.NUMBER,   # 2
            TokenType.NUMBER,   # 3
            TokenType.RPAREN,   # )
            TokenType.NUMBER,   # 4
            TokenType.RPAREN,   # )
            TokenType.EOF
        ]
        assert [t.type for t in tokens] == expected_types


class TestComments:
    """注释测试。"""

    def test_comment(self):
        """测试注释被正确跳过。"""
        lexer = Lexer("; This is a comment\\n42")
        tokens = lexer.tokenize()

        assert len(tokens) == 2  # NUMBER + EOF
        assert tokens[0].type == TokenType.NUMBER
        assert tokens[0].value == 42

    def test_inline_comment(self):
        """测试行内注释。"""
        lexer = Lexer("1 ; comment\\n2")
        tokens = lexer.tokenize()

        assert len(tokens) == 3  # NUMBER + NUMBER + EOF
        assert tokens[0].value == 1
        assert tokens[1].value == 2


class TestPositionTracking:
    """位置追踪测试。"""

    def test_position_tracking(self):
        """测试行号和列号追踪。"""
        lexer = Lexer("(+ 1\\n  2)")
        tokens = lexer.tokenize()

        # 第一行的 token
        assert tokens[0].line == 1  # (
        assert tokens[0].column == 1
        assert tokens[1].line == 1  # +
        assert tokens[2].line == 1  # 1

        # 第二行的 token
        assert tokens[3].line == 2  # 2


class TestEdgeCases:
    """边界情况测试。"""

    def test_minus_as_symbol(self):
        """测试单独的减号是符号。"""
        lexer = Lexer("-")
        tokens = lexer.tokenize()

        assert tokens[0].type == TokenType.SYMBOL
        assert tokens[0].value == "-"

    def test_define_expression(self):
        """测试 define 表达式。"""
        lexer = Lexer("(define x 42)")
        tokens = lexer.tokenize()

        assert tokens[1].type == TokenType.SYMBOL
        assert tokens[1].value == "define"
        assert tokens[2].type == TokenType.SYMBOL
        assert tokens[2].value == "x"
        assert tokens[3].type == TokenType.NUMBER
        assert tokens[3].value == 42`,
  readme: {
    zh: `# 模块 1：词法分析 (Lexer)

> "如何把字符串变成有意义的单元？"

## 问题引入

假设你收到一个字符串 \`"(+ 1 2)"\`，你需要理解它的含义。

**第一步**：把字符串分解成有意义的"单词"。

\`\`\`
"(+ 1 2)"  →  ["(", "+", "1", "2", ")"]
\`\`\`

这些"单词"在编译原理中叫做 **Token**（词法单元）。

**思考**：
- 空格去哪了？
- 如何区分 \`+\`（运算符）和 \`123\`（数字）？
- 如何处理 \`-42\`（负数）和 \`- 42\`（减法）？

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

\`\`\`
LPAREN   →  (
RPAREN   →  )
NUMBER   →  42, -10
SYMBOL   →  +, define, foo-bar
BOOLEAN  →  #t, #f
EOF      →  输入结束
\`\`\`

### Lexer 的工作流程

\`\`\`
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
\`\`\`

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

打开 \`skeleton.py\`，你会看到：

\`\`\`python
class Lexer:
    def __init__(self, source: str):
        self.source = source
        self.pos = 0
        self.line = 1
        self.column = 1

    def next_token(self) -> Token:
        # TODO: 实现这个方法
        pass
\`\`\`

### 步骤 2：实现核心方法

你需要实现以下方法：

1. \`current_char()\` - 返回当前字符
2. \`advance()\` - 前进一个字符
3. \`skip_whitespace()\` - 跳过空白字符
4. \`read_number()\` - 读取数字
5. \`read_symbol()\` - 读取符号
6. \`next_token()\` - 返回下一个 Token

### 步骤 3：运行测试

\`\`\`bash
cd tiny-interpreter
pytest learn/01-lexer/test_skeleton.py -v
\`\`\`

测试是渐进式的，从简单到复杂：
- \`test_empty_input\` - 空输入
- \`test_single_number\` - 单个数字
- \`test_parentheses\` - 括号
- \`test_simple_expression\` - 简单表达式
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

\`\`\`bash
$ pytest learn/01-lexer/test_skeleton.py -v
...
test_empty_input PASSED
test_single_number PASSED
test_simple_expression PASSED
...
\`\`\`

---

## 深入思考

完成实现后，思考这些问题：

1. 为什么要分开 \`current_char()\` 和 \`advance()\`？
2. 如何支持浮点数？需要修改哪些地方？
3. 如何支持字符串字面量（如 \`"hello"\`）？

这些问题在 [challenge.md](challenge.md) 中有更详细的讨论。

---

## 下一步

完成词法分析后，我们有了 Token 序列。

但 Token 序列是"扁平"的，我们需要把它组织成"树状"结构。

[进入模块 2：语法分析 →](../02-parser/README.md)`,
    en: `# Module 1: Lexer

> "How to turn strings into meaningful units?"

## Problem Introduction

Suppose you receive a string \`"(+ 1 2)"\`, and you need to understand its meaning.

**Step one**: Break the string into meaningful "words".

\`\`\`
"(+ 1 2)"  →  ["(", "+", "1", "2", ")"]
\`\`\`

These "words" are called **Tokens** in compiler theory.

**Think**:
- Where did the spaces go?
- How do you distinguish \`+\` (operator) from \`123\` (number)?
- How do you handle \`-42\` (negative number) vs \`- 42\` (subtraction)?

---

## Core Concepts

### What is a Token?

A Token is the smallest meaningful unit of source code. Each Token contains:

| Property | Description | Example |
|----------|-------------|---------|
| type | Token type | NUMBER, SYMBOL, LPAREN |
| value | Token value | 42, "+", "(" |
| line | Line number | 1 |
| column | Column number | 5 |

### Our Token Types

\`\`\`
LPAREN   →  (
RPAREN   →  )
NUMBER   →  42, -10
SYMBOL   →  +, define, foo-bar
BOOLEAN  →  #t, #f
EOF      →  End of input
\`\`\`

### How the Lexer Works

\`\`\`
Input: "(+ 1 2)"

Position:  0 1 2 3 4 5 6
Character: ( +   1   2 )

Steps:
  pos=0: see '(' → produce LPAREN
  pos=1: see '+' → produce SYMBOL("+")
  pos=2: see ' ' → skip whitespace
  pos=3: see '1' → produce NUMBER(1)
  pos=4: see ' ' → skip whitespace
  pos=5: see '2' → produce NUMBER(2)
  pos=6: see ')' → produce RPAREN
  pos=7: reached end → produce EOF

Output: [LPAREN, SYMBOL("+"), NUMBER(1), NUMBER(2), RPAREN, EOF]
\`\`\`

---

## Key Invariants

When implementing the Lexer, you must ensure:

1. **Completeness**: Every character is processed (either produces a Token or is skipped)
2. **Unambiguity**: Each character sequence can only be parsed as one type of Token
3. **Position accuracy**: Token line and column numbers must be correct
4. **Error handling**: Throw meaningful errors for illegal characters

---

## Hands-on Implementation

### Step 1: Understand the Skeleton Code

Open \`skeleton.py\`, you'll see:

\`\`\`python
class Lexer:
    def __init__(self, source: str):
        self.source = source
        self.pos = 0
        self.line = 1
        self.column = 1

    def next_token(self) -> Token:
        # TODO: Implement this method
        pass
\`\`\`

### Step 2: Implement Core Methods

You need to implement the following methods:

1. \`current_char()\` - Return the current character
2. \`advance()\` - Move forward one character
3. \`skip_whitespace()\` - Skip whitespace characters
4. \`read_number()\` - Read a number
5. \`read_symbol()\` - Read a symbol
6. \`next_token()\` - Return the next Token

### Step 3: Run Tests

\`\`\`bash
cd tiny-interpreter
pytest learn/01-lexer/test_skeleton.py -v
\`\`\`

Tests are progressive, from simple to complex:
- \`test_empty_input\` - Empty input
- \`test_single_number\` - Single number
- \`test_parentheses\` - Parentheses
- \`test_simple_expression\` - Simple expression
- ...

---

## Hints

If you get stuck, check the hints:

- [Hint 1: Basic approach](hints/hint1.md)
- [Hint 2: Handling numbers](hints/hint2.md)
- [Hint 3: Complete implementation approach](hints/hint3.md)

---

## Verify Success

When all tests pass, you've completed the lexer!

\`\`\`bash
$ pytest learn/01-lexer/test_skeleton.py -v
...
test_empty_input PASSED
test_single_number PASSED
test_simple_expression PASSED
...
\`\`\`

---

## Deep Thinking

After completing the implementation, think about these questions:

1. Why separate \`current_char()\` and \`advance()\`?
2. How would you support floating-point numbers? What changes are needed?
3. How would you support string literals (like \`"hello"\`)?

These questions are discussed in more detail in [challenge.md](challenge.md).

---

## Next Step

After completing lexical analysis, we have a Token sequence.

But the Token sequence is "flat" — we need to organize it into a "tree" structure.

[Enter Module 2: Parser →](../02-parser/README.md)`,
  },
  hints: [
    { level: 1, content: { zh: `# 提示 1：基本思路

## Lexer 的核心循环

Lexer 的工作可以概括为一个循环：

\`\`\`
while 还有字符:
    跳过空白
    跳过注释
    根据当前字符决定读取什么 Token
\`\`\`

## current_char() 和 advance()

这两个方法是基础：

\`\`\`python
def current_char(self) -> Optional[str]:
    # 检查是否到达末尾
    if self.pos >= len(self.source):
        return None
    return self.source[self.pos]

def advance(self) -> Optional[str]:
    char = self.current_char()
    if char is not None:
        self.pos += 1
        # 更新行列号
        if char == '\\n':
            self.line += 1
            self.column = 1
        else:
            self.column += 1
    return char
\`\`\`

## skip_whitespace()

\`\`\`python
def skip_whitespace(self):
    while self.current_char() and self.current_char().isspace():
        self.advance()
\`\`\`

## 下一步

理解了这些基础方法后，尝试实现 \`read_number()\` 和 \`read_symbol()\`。

如果还需要帮助，查看 [hint2.md](hint2.md)。`, en: `# Hint 1: Basic Approach

## The Lexer's Core Loop

The Lexer's work can be summarized as a loop:

\`\`\`
while there are characters:
    skip whitespace
    skip comments
    decide what Token to read based on current character
\`\`\`

## current_char() and advance()

These two methods are fundamental:

\`\`\`python
def current_char(self) -> Optional[str]:
    # Check if we've reached the end
    if self.pos >= len(self.source):
        return None
    return self.source[self.pos]

def advance(self) -> Optional[str]:
    char = self.current_char()
    if char is not None:
        self.pos += 1
        # Update line and column numbers
        if char == '\\n':
            self.line += 1
            self.column = 1
        else:
            self.column += 1
    return char
\`\`\`

## skip_whitespace()

\`\`\`python
def skip_whitespace(self):
    while self.current_char() and self.current_char().isspace():
        self.advance()
\`\`\`

## Next Step

After understanding these basic methods, try implementing \`read_number()\` and \`read_symbol()\`.

If you need more help, check [hint2.md](hint2.md).` } },
    { level: 2, content: { zh: `# 提示 2：处理数字和符号

## read_number()

读取数字的关键点：
1. 记录起始位置（用于错误报告）
2. 处理可能的负号
3. 收集所有数字字符
4. 转换为整数

\`\`\`python
def read_number(self) -> Token:
    start_line = self.line
    start_column = self.column
    num_str = ''

    # 处理负号
    if self.current_char() == '-':
        num_str += self.advance()

    # 收集数字
    while self.current_char() and self.current_char().isdigit():
        num_str += self.advance()

    return Token(TokenType.NUMBER, int(num_str), start_line, start_column)
\`\`\`

## read_symbol()

读取符号类似，但使用 \`is_symbol_char()\` 判断：

\`\`\`python
def read_symbol(self) -> Token:
    start_line = self.line
    start_column = self.column
    symbol = ''

    while self.current_char() and self.is_symbol_char(self.current_char()):
        symbol += self.advance()

    return Token(TokenType.SYMBOL, symbol, start_line, start_column)
\`\`\`

## read_boolean()

布尔值以 \`#\` 开头：

\`\`\`python
def read_boolean(self) -> Token:
    start_line = self.line
    start_column = self.column

    self.advance()  # 跳过 #
    char = self.current_char()

    if char == 't':
        self.advance()
        return Token(TokenType.BOOLEAN, True, start_line, start_column)
    elif char == 'f':
        self.advance()
        return Token(TokenType.BOOLEAN, False, start_line, start_column)
    else:
        raise LexerError(f"Invalid boolean: #{char}", start_line, start_column)
\`\`\`

## 下一步

现在你应该能实现 \`next_token()\` 了。

如果还需要帮助，查看 [hint3.md](hint3.md)。`, en: `# Hint 2: Handling Numbers and Symbols

## read_number()

Key points for reading numbers:
1. Record the starting position (for error reporting)
2. Handle possible negative sign
3. Collect all digit characters
4. Convert to integer

\`\`\`python
def read_number(self) -> Token:
    start_line = self.line
    start_column = self.column
    num_str = ''

    # Handle negative sign
    if self.current_char() == '-':
        num_str += self.advance()

    # Collect digits
    while self.current_char() and self.current_char().isdigit():
        num_str += self.advance()

    return Token(TokenType.NUMBER, int(num_str), start_line, start_column)
\`\`\`

## read_symbol()

Reading symbols is similar, but uses \`is_symbol_char()\` to check:

\`\`\`python
def read_symbol(self) -> Token:
    start_line = self.line
    start_column = self.column
    symbol = ''

    while self.current_char() and self.is_symbol_char(self.current_char()):
        symbol += self.advance()

    return Token(TokenType.SYMBOL, symbol, start_line, start_column)
\`\`\`

## read_boolean()

Booleans start with \`#\`:

\`\`\`python
def read_boolean(self) -> Token:
    start_line = self.line
    start_column = self.column

    self.advance()  # skip #
    char = self.current_char()

    if char == 't':
        self.advance()
        return Token(TokenType.BOOLEAN, True, start_line, start_column)
    elif char == 'f':
        self.advance()
        return Token(TokenType.BOOLEAN, False, start_line, start_column)
    else:
        raise LexerError(f"Invalid boolean: #{char}", start_line, start_column)
\`\`\`

## Next Step

Now you should be able to implement \`next_token()\`.

If you still need help, check [hint3.md](hint3.md).` } },
    { level: 3, content: { zh: `# 提示 3：完整的 next_token() 实现

## next_token() 的完整逻辑

\`\`\`python
def next_token(self) -> Token:
    # 1. 跳过空白
    self.skip_whitespace()

    # 2. 跳过注释（可能有多个连续注释）
    while self.current_char() == ';':
        self.skip_comment()
        self.skip_whitespace()

    # 3. 获取当前字符
    char = self.current_char()

    # 4. 检查是否到达末尾
    if char is None:
        return Token(TokenType.EOF, None, self.line, self.column)

    # 5. 括号
    if char == '(':
        token = Token(TokenType.LPAREN, '(', self.line, self.column)
        self.advance()
        return token

    if char == ')':
        token = Token(TokenType.RPAREN, ')', self.line, self.column)
        self.advance()
        return token

    # 6. 布尔值
    if char == '#':
        return self.read_boolean()

    # 7. 数字（包括负数）
    # 关键：负号后面必须紧跟数字才是负数
    if char.isdigit() or (char == '-' and self.peek_char() and self.peek_char().isdigit()):
        return self.read_number()

    # 8. 符号
    if self.is_symbol_char(char):
        return self.read_symbol()

    # 9. 无法识别的字符
    raise LexerError(f"Unexpected character: {char!r}", self.line, self.column)
\`\`\`

## 关键点

### 负数 vs 减号

\`\`\`python
# 这是负数：-42
# 这是减号：(- 4 2)
# 区别：负号后面紧跟数字

if char == '-' and self.peek_char() and self.peek_char().isdigit():
    return self.read_number()  # 负数
else:
    return self.read_symbol()  # 减号符号
\`\`\`

### skip_comment()

\`\`\`python
def skip_comment(self):
    if self.current_char() == ';':
        while self.current_char() and self.current_char() != '\\n':
            self.advance()
\`\`\`

## 完整实现

如果你还是卡住了，可以参考 \`src/tiny_interpreter/lexer.py\` 中的完整实现。

但建议先自己尝试，理解每一步的原因！`, en: `# Hint 3: Complete next_token() Implementation

## Complete Logic for next_token()

\`\`\`python
def next_token(self) -> Token:
    # 1. Skip whitespace
    self.skip_whitespace()

    # 2. Skip comments (there may be multiple consecutive comments)
    while self.current_char() == ';':
        self.skip_comment()
        self.skip_whitespace()

    # 3. Get current character
    char = self.current_char()

    # 4. Check if we've reached the end
    if char is None:
        return Token(TokenType.EOF, None, self.line, self.column)

    # 5. Parentheses
    if char == '(':
        token = Token(TokenType.LPAREN, '(', self.line, self.column)
        self.advance()
        return token

    if char == ')':
        token = Token(TokenType.RPAREN, ')', self.line, self.column)
        self.advance()
        return token

    # 6. Booleans
    if char == '#':
        return self.read_boolean()

    # 7. Numbers (including negative numbers)
    # Key: negative sign must be immediately followed by a digit
    if char.isdigit() or (char == '-' and self.peek_char() and self.peek_char().isdigit()):
        return self.read_number()

    # 8. Symbols
    if self.is_symbol_char(char):
        return self.read_symbol()

    # 9. Unrecognized character
    raise LexerError(f"Unexpected character: {char!r}", self.line, self.column)
\`\`\`

## Key Points

### Negative Numbers vs Minus Sign

\`\`\`python
# This is a negative number: -42
# This is a minus sign: (- 4 2)
# Difference: negative sign is immediately followed by a digit

if char == '-' and self.peek_char() and self.peek_char().isdigit():
    return self.read_number()  # negative number
else:
    return self.read_symbol()  # minus symbol
\`\`\`

### skip_comment()

\`\`\`python
def skip_comment(self):
    if self.current_char() == ';':
        while self.current_char() and self.current_char() != '\\n':
            self.advance()
\`\`\`

## Complete Implementation

If you're still stuck, you can refer to the complete implementation in \`src/tiny_interpreter/lexer.py\`.

But we recommend trying on your own first and understanding the reason behind each step!` } },
  ],
};
