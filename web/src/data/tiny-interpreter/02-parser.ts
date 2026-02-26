import type { LearningModule } from "../types";

export const module02Parser: LearningModule = {
  id: "02-parser",
  index: 2,
  slug: "parser",
  hasCode: true,
  vizType: "ast",
  diagnostics: [
    { pattern: "_allFail", message: { zh: "先实现 `current_token()` 和 `advance()`——它们是最基础的", en: "Start with `current_token()` and `advance()` — they're the foundation" } },
    { pattern: ["test_parse_empty_list", "test_parse_simple_expression"], message: { zh: "`parse_sexp()` 需要循环读取元素直到遇到 `)`", en: "`parse_sexp()` needs to loop reading elements until it hits `)`" } },
    { pattern: "test_parse_nested_expression", message: { zh: "嵌套表达式需要递归——`parse_expr()` 遇到 `(` 时应该调用 `parse_sexp()`", en: "Nested expressions need recursion — `parse_expr()` should call `parse_sexp()` when it sees `(`" } },
    { pattern: "test_unclosed_paren", message: { zh: "没有处理未闭合括号？在循环里检查 EOF", en: "Not handling unclosed parens? Check for EOF inside the loop" } },
  ],
  skeleton: `"""语法分析器骨架代码。

你的任务是实现标记为 TODO 的方法。

运行测试：
    pytest learn/02-parser/test_skeleton.py -v

依赖：
    这个模块依赖 01-lexer 的实现。
    如果你还没完成 lexer，可以使用参考实现：
    from src.tiny_interpreter.lexer import Lexer, Token, TokenType
"""

from dataclasses import dataclass
from typing import List, Union

# 如果你完成了 01-lexer，使用你的实现：
# from learn.lexer.skeleton import Lexer, Token, TokenType

# 否则使用参考实现：
import sys
import os
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..'))
from src.tiny_interpreter.lexer import Lexer, Token, TokenType


# ============================================================
# AST 节点定义
# ============================================================

@dataclass
class Number:
    """数字节点。"""
    value: int
    line: int
    column: int

    def __repr__(self):
        return f"Number({self.value})"


@dataclass
class Boolean:
    """布尔值节点。"""
    value: bool
    line: int
    column: int

    def __repr__(self):
        return f"Boolean({self.value})"


@dataclass
class Symbol:
    """符号节点。"""
    name: str
    line: int
    column: int

    def __repr__(self):
        return f"Symbol({self.name!r})"


@dataclass
class SExpression:
    """S-表达式节点（列表）。"""
    elements: List['ASTNode']
    line: int
    column: int

    def __repr__(self):
        return f"SExpression({self.elements})"


# 类型别名
ASTNode = Union[Number, Boolean, Symbol, SExpression]


# ============================================================
# 错误类
# ============================================================

class ParserError(Exception):
    """语法分析错误。"""
    def __init__(self, message: str, line: int, column: int):
        self.message = message
        self.line = line
        self.column = column
        super().__init__(f"{message} at line {line}, column {column}")


# ============================================================
# Parser 类
# ============================================================

class Parser:
    """语法分析器：将 Token 序列转换为 AST。

    使用方法：
        lexer = Lexer("(+ 1 2)")
        tokens = lexer.tokenize()
        parser = Parser(tokens)
        ast = parser.parse()
    """

    def __init__(self, tokens: List[Token]):
        """初始化 Parser。

        Args:
            tokens: Token 列表（来自 Lexer）
        """
        self.tokens = tokens
        self.pos = 0

    def current_token(self) -> Token:
        """返回当前位置的 Token。

        TODO: 实现这个方法

        提示：
        - 如果 pos 超出范围，返回最后一个 Token（应该是 EOF）
        - 否则返回 tokens[pos]
        """
        # TODO: 实现
        pass

    def advance(self) -> Token:
        """消费当前 Token 并前进到下一个。

        TODO: 实现这个方法

        提示：
        - 获取当前 Token
        - 如果不是最后一个，pos 加 1
        - 返回消费的 Token
        """
        # TODO: 实现
        pass

    def expect(self, token_type: TokenType) -> Token:
        """期望当前 Token 是特定类型，并消费它。

        TODO: 实现这个方法

        Args:
            token_type: 期望的 Token 类型

        Returns:
            消费的 Token

        Raises:
            ParserError: 如果当前 Token 类型不匹配

        提示：
        - 获取当前 Token
        - 检查类型是否匹配
        - 如果不匹配，抛出 ParserError
        - 如果匹配，调用 advance() 并返回
        """
        # TODO: 实现
        pass

    def parse_atom(self) -> ASTNode:
        """解析原子表达式（数字、布尔值、符号）。

        TODO: 实现这个方法

        提示：
        - 获取当前 Token
        - 根据 Token 类型创建对应的 AST 节点
        - NUMBER → Number
        - BOOLEAN → Boolean
        - SYMBOL → Symbol
        - 其他类型 → 抛出 ParserError
        - 记得调用 advance() 消费 Token
        """
        # TODO: 实现
        pass

    def parse_sexp(self) -> SExpression:
        """解析 S-表达式（列表）。

        TODO: 实现这个方法

        S-表达式的形式：(元素1 元素2 ...)

        提示：
        - 用 expect(LPAREN) 消费左括号，记录位置
        - 创建空列表存储元素
        - 循环：只要当前不是 RPAREN
          - 检查是否意外到达 EOF（抛出错误）
          - 调用 parse_expr() 解析一个元素
          - 添加到列表
        - 用 expect(RPAREN) 消费右括号
        - 返回 SExpression 节点
        """
        # TODO: 实现
        pass

    def parse_expr(self) -> ASTNode:
        """解析一个表达式。

        TODO: 实现这个方法

        表达式可以是：
        - 原子（数字、布尔值、符号）
        - S-表达式（以 '(' 开头的列表）

        提示：
        - 检查当前 Token 类型
        - 如果是 LPAREN，调用 parse_sexp()
        - 否则调用 parse_atom()
        """
        # TODO: 实现
        pass

    def parse(self) -> List[ASTNode]:
        """解析所有表达式。

        Returns:
            AST 节点列表
        """
        expressions = []
        while self.current_token().type != TokenType.EOF:
            expressions.append(self.parse_expr())
        return expressions


# ============================================================
# 便捷函数
# ============================================================

def parse(source: str) -> List[ASTNode]:
    """便捷函数：直接从源代码解析到 AST。

    Args:
        source: 源代码字符串

    Returns:
        AST 节点列表
    """
    lexer = Lexer(source)
    tokens = lexer.tokenize()
    parser = Parser(tokens)
    return parser.parse()`,
  testCode: `"""语法分析器测试。

运行测试：
    pytest learn/02-parser/test_skeleton.py -v
"""

import pytest
from skeleton import Parser, parse, Number, Boolean, Symbol, SExpression, ParserError

import sys
import os
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..'))
from src.tiny_interpreter.lexer import Lexer, TokenType


class TestAtoms:
    """原子表达式测试。"""

    def test_parse_number(self):
        """测试解析数字。"""
        ast = parse("42")

        assert len(ast) == 1
        assert isinstance(ast[0], Number)
        assert ast[0].value == 42

    def test_parse_negative_number(self):
        """测试解析负数。"""
        ast = parse("-42")

        assert len(ast) == 1
        assert isinstance(ast[0], Number)
        assert ast[0].value == -42

    def test_parse_boolean_true(self):
        """测试解析 #t。"""
        ast = parse("#t")

        assert len(ast) == 1
        assert isinstance(ast[0], Boolean)
        assert ast[0].value is True

    def test_parse_boolean_false(self):
        """测试解析 #f。"""
        ast = parse("#f")

        assert len(ast) == 1
        assert isinstance(ast[0], Boolean)
        assert ast[0].value is False

    def test_parse_symbol(self):
        """测试解析符号。"""
        ast = parse("foo")

        assert len(ast) == 1
        assert isinstance(ast[0], Symbol)
        assert ast[0].name == "foo"

    def test_parse_operator_symbol(self):
        """测试解析运算符符号。"""
        ast = parse("+")

        assert len(ast) == 1
        assert isinstance(ast[0], Symbol)
        assert ast[0].name == "+"


class TestSExpressions:
    """S-表达式测试。"""

    def test_parse_empty_list(self):
        """测试解析空列表。"""
        ast = parse("()")

        assert len(ast) == 1
        assert isinstance(ast[0], SExpression)
        assert len(ast[0].elements) == 0

    def test_parse_simple_expression(self):
        """测试解析简单表达式 (+ 1 2)。"""
        ast = parse("(+ 1 2)")

        assert len(ast) == 1
        sexp = ast[0]
        assert isinstance(sexp, SExpression)
        assert len(sexp.elements) == 3

        assert isinstance(sexp.elements[0], Symbol)
        assert sexp.elements[0].name == "+"

        assert isinstance(sexp.elements[1], Number)
        assert sexp.elements[1].value == 1

        assert isinstance(sexp.elements[2], Number)
        assert sexp.elements[2].value == 2

    def test_parse_nested_expression(self):
        """测试解析嵌套表达式 (+ 1 (* 2 3))。"""
        ast = parse("(+ 1 (* 2 3))")

        assert len(ast) == 1
        outer = ast[0]
        assert isinstance(outer, SExpression)
        assert len(outer.elements) == 3

        # 第一个元素是 +
        assert isinstance(outer.elements[0], Symbol)
        assert outer.elements[0].name == "+"

        # 第二个元素是 1
        assert isinstance(outer.elements[1], Number)

        # 第三个元素是嵌套的 S-表达式
        inner = outer.elements[2]
        assert isinstance(inner, SExpression)
        assert len(inner.elements) == 3
        assert inner.elements[0].name == "*"
        assert inner.elements[1].value == 2
        assert inner.elements[2].value == 3


class TestMultipleExpressions:
    """多表达式测试。"""

    def test_parse_multiple_atoms(self):
        """测试解析多个原子。"""
        ast = parse("1 2 3")

        assert len(ast) == 3
        assert all(isinstance(node, Number) for node in ast)
        assert [node.value for node in ast] == [1, 2, 3]

    def test_parse_multiple_expressions(self):
        """测试解析多个表达式。"""
        ast = parse("(+ 1 2) (- 3 4)")

        assert len(ast) == 2
        assert all(isinstance(node, SExpression) for node in ast)


class TestDefineAndLambda:
    """define 和 lambda 表达式测试。"""

    def test_parse_define(self):
        """测试解析 define 表达式。"""
        ast = parse("(define x 42)")

        assert len(ast) == 1
        sexp = ast[0]
        assert isinstance(sexp, SExpression)
        assert len(sexp.elements) == 3

        assert sexp.elements[0].name == "define"
        assert sexp.elements[1].name == "x"
        assert sexp.elements[2].value == 42

    def test_parse_lambda(self):
        """测试解析 lambda 表达式。"""
        ast = parse("(lambda (x) (* x x))")

        assert len(ast) == 1
        sexp = ast[0]
        assert isinstance(sexp, SExpression)

        # lambda
        assert sexp.elements[0].name == "lambda"

        # 参数列表 (x)
        params = sexp.elements[1]
        assert isinstance(params, SExpression)
        assert len(params.elements) == 1
        assert params.elements[0].name == "x"

        # 函数体 (* x x)
        body = sexp.elements[2]
        assert isinstance(body, SExpression)
        assert body.elements[0].name == "*"


class TestErrors:
    """错误处理测试。"""

    def test_unclosed_paren(self):
        """测试未闭合的括号。"""
        with pytest.raises(ParserError):
            parse("(+ 1 2")

    def test_unexpected_token(self):
        """测试意外的 Token。"""
        lexer = Lexer(")")
        tokens = lexer.tokenize()
        parser = Parser(tokens)

        with pytest.raises(ParserError):
            parser.parse_expr()


class TestPositionTracking:
    """位置追踪测试。"""

    def test_position_in_ast(self):
        """测试 AST 节点包含正确的位置信息。"""
        ast = parse("(+ 1\\n  2)")

        sexp = ast[0]
        assert sexp.line == 1
        assert sexp.column == 1

        # 数字 2 在第二行
        num2 = sexp.elements[2]
        assert num2.line == 2`,
  readme: {
    zh: `# 模块 2：语法分析 (Parser)

> "如何把 Token 组织成树状结构？"

## 问题引入

词法分析给了我们一个 Token 序列：

\`\`\`
[LPAREN, SYMBOL("+"), NUMBER(1), LPAREN, SYMBOL("*"), NUMBER(2), NUMBER(3), RPAREN, RPAREN]
\`\`\`

这是"扁平"的。但表达式 \`(+ 1 (* 2 3))\` 有层次结构：

\`\`\`
    +
   / \\
  1   *
     / \\
    2   3
\`\`\`

**问题**：如何从扁平的 Token 序列构建这棵树？

---

## 核心概念

### 抽象语法树 (AST)

AST 是程序的树状表示。每个节点代表一个语法结构：

| 节点类型 | 说明 | 示例 |
|---------|------|------|
| Number | 数字字面量 | \`42\` |
| Boolean | 布尔字面量 | \`#t\` |
| Symbol | 符号 | \`+\`, \`define\` |
| SExpression | S-表达式（列表） | \`(+ 1 2)\` |

### S-表达式

Lisp 的语法非常简单：一切都是 S-表达式。

\`\`\`
S-表达式 = 原子 | (S-表达式*)

原子 = 数字 | 布尔值 | 符号
\`\`\`

这意味着：
- \`42\` 是 S-表达式（原子）
- \`(+ 1 2)\` 是 S-表达式（列表）
- \`(+ 1 (* 2 3))\` 是 S-表达式（嵌套列表）

### 递归下降解析

我们使用**递归下降**方法解析：

\`\`\`
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
\`\`\`

关键洞察：\`parse_expr()\` 调用 \`parse_sexp()\`，而 \`parse_sexp()\` 又调用 \`parse_expr()\`。这种**相互递归**自然地处理了嵌套结构。

---

## 关键不变量

1. **括号匹配**：每个 \`(\` 必须有对应的 \`)\`
2. **完整消费**：解析完成后，所有 Token 都被处理（除了 EOF）
3. **结构正确**：AST 的结构反映源代码的嵌套关系

---

## 动手实现

### 步骤 1：理解骨架代码

打开 \`skeleton.py\`，你会看到：

\`\`\`python
class Parser:
    def __init__(self, tokens: List[Token]):
        self.tokens = tokens
        self.pos = 0

    def parse_expr(self) -> ASTNode:
        # TODO: 实现
        pass
\`\`\`

### 步骤 2：实现核心方法

你需要实现：

1. \`current_token()\` - 返回当前 Token
2. \`advance()\` - 前进到下一个 Token
3. \`expect(token_type)\` - 期望特定类型的 Token
4. \`parse_atom()\` - 解析原子（数字、布尔值、符号）
5. \`parse_sexp()\` - 解析 S-表达式
6. \`parse_expr()\` - 解析表达式（原子或 S-表达式）

### 步骤 3：运行测试

\`\`\`bash
cd tiny-interpreter
pytest learn/02-parser/test_skeleton.py -v
\`\`\`

---

## 可视化

让我们看看 \`(+ 1 (* 2 3))\` 是如何被解析的：

\`\`\`
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
\`\`\`

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

[进入模块 3：环境模型 →](../03-environment/README.md)`,
    en: `# Module 2: Parser

> "How to organize Tokens into a tree structure?"

## Problem Introduction

Lexical analysis gave us a Token sequence:

\`\`\`
[LPAREN, SYMBOL("+"), NUMBER(1), LPAREN, SYMBOL("*"), NUMBER(2), NUMBER(3), RPAREN, RPAREN]
\`\`\`

This is "flat". But the expression \`(+ 1 (* 2 3))\` has a hierarchical structure:

\`\`\`
    +
   / \\
  1   *
     / \\
    2   3
\`\`\`

**Problem**: How do we build this tree from a flat Token sequence?

---

## Core Concepts

### Abstract Syntax Tree (AST)

An AST is a tree representation of a program. Each node represents a syntactic structure:

| Node Type | Description | Example |
|-----------|-------------|---------|
| Number | Numeric literal | \`42\` |
| Boolean | Boolean literal | \`#t\` |
| Symbol | Symbol | \`+\`, \`define\` |
| SExpression | S-expression (list) | \`(+ 1 2)\` |

### S-Expressions

Lisp's syntax is very simple: everything is an S-expression.

\`\`\`
S-expression = atom | (S-expression*)

atom = number | boolean | symbol
\`\`\`

This means:
- \`42\` is an S-expression (atom)
- \`(+ 1 2)\` is an S-expression (list)
- \`(+ 1 (* 2 3))\` is an S-expression (nested list)

### Recursive Descent Parsing

We use a **recursive descent** approach to parse:

\`\`\`
parse_expr():
    if current is '(':
        return parse_sexp()  # parse list
    else:
        return parse_atom()  # parse atom

parse_sexp():
    expect '('
    elements = []
    while current is not ')':
        elements.append(parse_expr())  # recursion!
    expect ')'
    return SExpression(elements)
\`\`\`

Key insight: \`parse_expr()\` calls \`parse_sexp()\`, and \`parse_sexp()\` calls \`parse_expr()\`. This **mutual recursion** naturally handles nested structures.

---

## Key Invariants

1. **Parenthesis matching**: Every \`(\` must have a corresponding \`)\`
2. **Complete consumption**: After parsing, all Tokens are processed (except EOF)
3. **Structural correctness**: The AST structure reflects the nesting in the source code

---

## Hands-on Implementation

### Step 1: Understand the Skeleton Code

Open \`skeleton.py\`, you'll see:

\`\`\`python
class Parser:
    def __init__(self, tokens: List[Token]):
        self.tokens = tokens
        self.pos = 0

    def parse_expr(self) -> ASTNode:
        # TODO: Implement
        pass
\`\`\`

### Step 2: Implement Core Methods

You need to implement:

1. \`current_token()\` - Return the current Token
2. \`advance()\` - Move to the next Token
3. \`expect(token_type)\` - Expect a specific Token type
4. \`parse_atom()\` - Parse atoms (numbers, booleans, symbols)
5. \`parse_sexp()\` - Parse S-expressions
6. \`parse_expr()\` - Parse expressions (atoms or S-expressions)

### Step 3: Run Tests

\`\`\`bash
cd tiny-interpreter
pytest learn/02-parser/test_skeleton.py -v
\`\`\`

---

## Visualization

Let's see how \`(+ 1 (* 2 3))\` is parsed:

\`\`\`
Token sequence: ( + 1 ( * 2 3 ) )
                0 1 2 3 4 5 6 7 8

parse_expr() at pos=0
  see '(' → call parse_sexp()
    expect '(' ✓, pos=1
    parse_expr() at pos=1
      see '+' → call parse_atom()
        return Symbol('+'), pos=2
    parse_expr() at pos=2
      see '1' → call parse_atom()
        return Number(1), pos=3
    parse_expr() at pos=3
      see '(' → call parse_sexp()
        expect '(' ✓, pos=4
        parse_expr() at pos=4
          return Symbol('*'), pos=5
        parse_expr() at pos=5
          return Number(2), pos=6
        parse_expr() at pos=6
          return Number(3), pos=7
        expect ')' ✓, pos=8
        return SExpression([Symbol('*'), Number(2), Number(3)])
    expect ')' ✓, pos=9
    return SExpression([Symbol('+'), Number(1), SExpression(...)])
\`\`\`

---

## Hints

If you get stuck, check the hints:

- [Hint 1: Basic structure](hints/hint1.md)
- [Hint 2: Recursive parsing](hints/hint2.md)
- [Hint 3: Complete implementation](hints/hint3.md)

---

## Deep Thinking

After completing the implementation, think about these questions (see [challenge.md](challenge.md)):

1. What happens if parentheses don't match?
2. How can you provide better error messages?
3. What are the limitations of this parsing approach?

---

## Next Step

Now we have an AST, but we can't execute it yet.

Execution requires knowing where variable values are stored.

[Enter Module 3: Environment Model →](../03-environment/README.md)`,
  },
  hints: [
    { level: 1, content: { zh: `# 提示 1：基本结构

## Parser 的核心方法

Parser 需要几个基础方法来操作 Token 流：

\`\`\`python
def current_token(self) -> Token:
    """返回当前 Token。"""
    if self.pos >= len(self.tokens):
        return self.tokens[-1]  # 返回 EOF
    return self.tokens[self.pos]

def advance(self) -> Token:
    """消费当前 Token 并前进。"""
    token = self.current_token()
    if self.pos < len(self.tokens) - 1:
        self.pos += 1
    return token
\`\`\`

## expect() 方法

这个方法用于确保当前 Token 是期望的类型：

\`\`\`python
def expect(self, token_type: TokenType) -> Token:
    token = self.current_token()
    if token.type != token_type:
        raise ParserError(
            f"Expected {token_type.name}, got {token.type.name}",
            token.line,
            token.column
        )
    return self.advance()
\`\`\`

## 下一步

理解了这些基础方法后，尝试实现 \`parse_atom()\`。

如果还需要帮助，查看 [hint2.md](hint2.md)。`, en: `# Hint 1: Basic Structure

## Parser's Core Methods

The Parser needs a few basic methods to operate on the Token stream:

\`\`\`python
def current_token(self) -> Token:
    """Return the current Token."""
    if self.pos >= len(self.tokens):
        return self.tokens[-1]  # Return EOF
    return self.tokens[self.pos]

def advance(self) -> Token:
    """Consume the current Token and advance."""
    token = self.current_token()
    if self.pos < len(self.tokens) - 1:
        self.pos += 1
    return token
\`\`\`

## The expect() Method

This method ensures the current Token is the expected type:

\`\`\`python
def expect(self, token_type: TokenType) -> Token:
    token = self.current_token()
    if token.type != token_type:
        raise ParserError(
            f"Expected {token_type.name}, got {token.type.name}",
            token.line,
            token.column
        )
    return self.advance()
\`\`\`

## Next Step

After understanding these basic methods, try implementing \`parse_atom()\`.

If you need more help, check [hint2.md](hint2.md).` } },
    { level: 2, content: { zh: `# 提示 2：递归解析

## parse_atom()

解析原子很直接——根据 Token 类型创建对应的 AST 节点：

\`\`\`python
def parse_atom(self) -> ASTNode:
    token = self.current_token()

    if token.type == TokenType.NUMBER:
        self.advance()
        return Number(token.value, token.line, token.column)

    if token.type == TokenType.BOOLEAN:
        self.advance()
        return Boolean(token.value, token.line, token.column)

    if token.type == TokenType.SYMBOL:
        self.advance()
        return Symbol(token.value, token.line, token.column)

    raise ParserError(
        f"Unexpected token: {token.type.name}",
        token.line,
        token.column
    )
\`\`\`

## parse_expr()

这是入口点——决定调用 \`parse_atom()\` 还是 \`parse_sexp()\`：

\`\`\`python
def parse_expr(self) -> ASTNode:
    token = self.current_token()

    if token.type == TokenType.LPAREN:
        return self.parse_sexp()
    else:
        return self.parse_atom()
\`\`\`

## 下一步

现在尝试实现 \`parse_sexp()\`。这是最关键的部分！

如果还需要帮助，查看 [hint3.md](hint3.md)。`, en: `# Hint 2: Recursive Parsing

## parse_atom()

Parsing atoms is straightforward — create the corresponding AST node based on Token type:

\`\`\`python
def parse_atom(self) -> ASTNode:
    token = self.current_token()

    if token.type == TokenType.NUMBER:
        self.advance()
        return Number(token.value, token.line, token.column)

    if token.type == TokenType.BOOLEAN:
        self.advance()
        return Boolean(token.value, token.line, token.column)

    if token.type == TokenType.SYMBOL:
        self.advance()
        return Symbol(token.value, token.line, token.column)

    raise ParserError(
        f"Unexpected token: {token.type.name}",
        token.line,
        token.column
    )
\`\`\`

## parse_expr()

This is the entry point — decides whether to call \`parse_atom()\` or \`parse_sexp()\`:

\`\`\`python
def parse_expr(self) -> ASTNode:
    token = self.current_token()

    if token.type == TokenType.LPAREN:
        return self.parse_sexp()
    else:
        return self.parse_atom()
\`\`\`

## Next Step

Now try implementing \`parse_sexp()\`. This is the most critical part!

If you need more help, check [hint3.md](hint3.md).` } },
    { level: 3, content: { zh: `# 提示 3：完整的 parse_sexp() 实现

## parse_sexp() 的完整逻辑

\`\`\`python
def parse_sexp(self) -> SExpression:
    # 1. 消费左括号，记录位置
    lparen = self.expect(TokenType.LPAREN)

    # 2. 收集元素
    elements = []
    while self.current_token().type != TokenType.RPAREN:
        # 检查是否意外到达文件末尾
        if self.current_token().type == TokenType.EOF:
            raise ParserError(
                "Unexpected EOF, expected ')'",
                self.current_token().line,
                self.current_token().column
            )
        # 递归解析元素
        elements.append(self.parse_expr())

    # 3. 消费右括号
    self.expect(TokenType.RPAREN)

    # 4. 返回 S-表达式节点
    return SExpression(elements, lparen.line, lparen.column)
\`\`\`

## 关键点

### 递归的魔力

注意 \`parse_sexp()\` 调用 \`parse_expr()\`，而 \`parse_expr()\` 可能又调用 \`parse_sexp()\`。

这种相互递归自然地处理了任意深度的嵌套：

\`\`\`
(+ 1 (* 2 (- 3 4)))
\`\`\`

### 错误处理

两个重要的错误情况：
1. 期望 \`)\` 但遇到 EOF → 括号未闭合
2. 期望原子但遇到 \`)\` → 意外的右括号

## 完整实现

如果你还是卡住了，可以参考 \`src/tiny_interpreter/parser.py\` 中的完整实现。

但建议先自己尝试，理解递归下降的工作原理！`, en: `# Hint 3: Complete parse_sexp() Implementation

## Complete Logic for parse_sexp()

\`\`\`python
def parse_sexp(self) -> SExpression:
    # 1. Consume left parenthesis, record position
    lparen = self.expect(TokenType.LPAREN)

    # 2. Collect elements
    elements = []
    while self.current_token().type != TokenType.RPAREN:
        # Check for unexpected end of file
        if self.current_token().type == TokenType.EOF:
            raise ParserError(
                "Unexpected EOF, expected ')'",
                self.current_token().line,
                self.current_token().column
            )
        # Recursively parse element
        elements.append(self.parse_expr())

    # 3. Consume right parenthesis
    self.expect(TokenType.RPAREN)

    # 4. Return S-expression node
    return SExpression(elements, lparen.line, lparen.column)
\`\`\`

## Key Points

### The Magic of Recursion

Notice that \`parse_sexp()\` calls \`parse_expr()\`, and \`parse_expr()\` may call \`parse_sexp()\` again.

This mutual recursion naturally handles arbitrary nesting depth:

\`\`\`
(+ 1 (* 2 (- 3 4)))
\`\`\`

### Error Handling

Two important error cases:
1. Expected \`)\` but encountered EOF → unclosed parenthesis
2. Expected atom but encountered \`)\` → unexpected right parenthesis

## Complete Implementation

If you're still stuck, you can refer to the complete implementation in \`src/tiny_interpreter/parser.py\`.

But we recommend trying on your own first and understanding how recursive descent works!` } },
  ],
};
