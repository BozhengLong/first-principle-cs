"""基础求值器测试。

运行测试：
    pytest learn/04-evaluator-basic/test_skeleton.py -v
"""

import pytest
from skeleton import Evaluator, EvaluatorError


@pytest.fixture
def evaluator():
    """创建求值器实例。"""
    return Evaluator()


class TestSelfEvaluating:
    """自求值表达式测试。"""

    def test_number(self, evaluator):
        """测试数字求值。"""
        assert evaluator.run("42") == 42
        assert evaluator.run("-10") == -10

    def test_boolean(self, evaluator):
        """测试布尔值求值。"""
        assert evaluator.run("#t") is True
        assert evaluator.run("#f") is False


class TestArithmetic:
    """算术运算测试。"""

    def test_addition(self, evaluator):
        """测试加法。"""
        assert evaluator.run("(+ 1 2)") == 3
        assert evaluator.run("(+ 1 2 3)") == 6

    def test_subtraction(self, evaluator):
        """测试减法。"""
        assert evaluator.run("(- 5 3)") == 2

    def test_multiplication(self, evaluator):
        """测试乘法。"""
        assert evaluator.run("(* 3 4)") == 12

    def test_division(self, evaluator):
        """测试除法（整数除法）。"""
        assert evaluator.run("(/ 10 3)") == 3

    def test_nested_arithmetic(self, evaluator):
        """测试嵌套算术。"""
        assert evaluator.run("(+ 1 (* 2 3))") == 7
        assert evaluator.run("(* (+ 1 2) (- 5 3))") == 6


class TestComparison:
    """比较运算测试。"""

    def test_equal(self, evaluator):
        """测试相等。"""
        assert evaluator.run("(= 1 1)") is True
        assert evaluator.run("(= 1 2)") is False

    def test_less_than(self, evaluator):
        """测试小于。"""
        assert evaluator.run("(< 1 2)") is True
        assert evaluator.run("(< 2 1)") is False

    def test_greater_than(self, evaluator):
        """测试大于。"""
        assert evaluator.run("(> 2 1)") is True
        assert evaluator.run("(> 1 2)") is False


class TestDefine:
    """define 测试。"""

    def test_define_number(self, evaluator):
        """测试定义数字变量。"""
        evaluator.run("(define x 42)")
        assert evaluator.run("x") == 42

    def test_define_expression(self, evaluator):
        """测试定义表达式结果。"""
        evaluator.run("(define x (+ 1 2))")
        assert evaluator.run("x") == 3

    def test_use_defined_variable(self, evaluator):
        """测试使用定义的变量。"""
        evaluator.run("(define x 10)")
        evaluator.run("(define y 20)")
        assert evaluator.run("(+ x y)") == 30


class TestIf:
    """if 测试。"""

    def test_if_true(self, evaluator):
        """测试条件为真。"""
        assert evaluator.run("(if #t 1 2)") == 1

    def test_if_false(self, evaluator):
        """测试条件为假。"""
        assert evaluator.run("(if #f 1 2)") == 2

    def test_if_with_comparison(self, evaluator):
        """测试带比较的条件。"""
        assert evaluator.run("(if (< 1 2) 10 20)") == 10
        assert evaluator.run("(if (> 1 2) 10 20)") == 20

    def test_if_nested(self, evaluator):
        """测试嵌套 if。"""
        code = "(if #t (if #f 1 2) 3)"
        assert evaluator.run(code) == 2


class TestQuote:
    """quote 测试。"""

    def test_quote_number(self, evaluator):
        """测试引用数字。"""
        assert evaluator.run("(quote 42)") == 42

    def test_quote_symbol(self, evaluator):
        """测试引用符号。"""
        assert evaluator.run("(quote x)") == "x"

    def test_quote_list(self, evaluator):
        """测试引用列表。"""
        assert evaluator.run("(quote (1 2 3))") == [1, 2, 3]

    def test_quote_nested(self, evaluator):
        """测试引用嵌套列表。"""
        assert evaluator.run("(quote (+ 1 2))") == ["+", 1, 2]


class TestBegin:
    """begin 测试。"""

    def test_begin_single(self, evaluator):
        """测试单个表达式。"""
        assert evaluator.run("(begin 42)") == 42

    def test_begin_multiple(self, evaluator):
        """测试多个表达式。"""
        assert evaluator.run("(begin 1 2 3)") == 3

    def test_begin_with_define(self, evaluator):
        """测试带 define 的 begin。"""
        result = evaluator.run("""
            (begin
                (define x 10)
                (define y 20)
                (+ x y))
        """)
        assert result == 30


class TestListOperations:
    """列表操作测试。"""

    def test_list(self, evaluator):
        """测试创建列表。"""
        assert evaluator.run("(list 1 2 3)") == [1, 2, 3]

    def test_car(self, evaluator):
        """测试 car。"""
        assert evaluator.run("(car (list 1 2 3))") == 1

    def test_cdr(self, evaluator):
        """测试 cdr。"""
        assert evaluator.run("(cdr (list 1 2 3))") == [2, 3]

    def test_cons(self, evaluator):
        """测试 cons。"""
        assert evaluator.run("(cons 1 (list 2 3))") == [1, 2, 3]

    def test_null(self, evaluator):
        """测试 null?。"""
        assert evaluator.run("(null? (list))") is True
        assert evaluator.run("(null? (list 1))") is False


class TestTypePredicates:
    """类型判断测试。"""

    def test_number_predicate(self, evaluator):
        """测试 number?。"""
        assert evaluator.run("(number? 42)") is True
        assert evaluator.run("(number? #t)") is False

    def test_boolean_predicate(self, evaluator):
        """测试 boolean?。"""
        assert evaluator.run("(boolean? #t)") is True
        assert evaluator.run("(boolean? 42)") is False

    def test_list_predicate(self, evaluator):
        """测试 list?。"""
        assert evaluator.run("(list? (list 1 2))") is True
        assert evaluator.run("(list? 42)") is False


class TestEmptyList:
    """空列表测试。"""

    def test_empty_sexp(self, evaluator):
        """测试空 S-表达式。"""
        assert evaluator.run("()") == []
