"""语法分析器骨架代码。

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
    return parser.parse()
