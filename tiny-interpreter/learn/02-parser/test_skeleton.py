"""语法分析器测试。

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
        ast = parse("(+ 1\n  2)")

        sexp = ast[0]
        assert sexp.line == 1
        assert sexp.column == 1

        # 数字 2 在第二行
        num2 = sexp.elements[2]
        assert num2.line == 2
