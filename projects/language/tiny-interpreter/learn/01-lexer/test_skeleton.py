"""词法分析器测试。

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
        lexer = Lexer("   \n\t  ")
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
        lexer = Lexer("; This is a comment\n42")
        tokens = lexer.tokenize()

        assert len(tokens) == 2  # NUMBER + EOF
        assert tokens[0].type == TokenType.NUMBER
        assert tokens[0].value == 42

    def test_inline_comment(self):
        """测试行内注释。"""
        lexer = Lexer("1 ; comment\n2")
        tokens = lexer.tokenize()

        assert len(tokens) == 3  # NUMBER + NUMBER + EOF
        assert tokens[0].value == 1
        assert tokens[1].value == 2


class TestPositionTracking:
    """位置追踪测试。"""

    def test_position_tracking(self):
        """测试行号和列号追踪。"""
        lexer = Lexer("(+ 1\n  2)")
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
        assert tokens[3].value == 42
