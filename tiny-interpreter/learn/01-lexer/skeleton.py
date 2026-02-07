"""词法分析器骨架代码。

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
        - 如果字符是换行符 '\n'，行号加 1，列号重置为 1
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
        return tokens
