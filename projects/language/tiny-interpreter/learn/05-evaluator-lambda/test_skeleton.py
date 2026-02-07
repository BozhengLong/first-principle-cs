"""Lambda 与闭包测试。

运行测试：
    pytest learn/05-evaluator-lambda/test_skeleton.py -v
"""

import pytest
from skeleton import Evaluator, Closure, EvaluatorError


@pytest.fixture
def evaluator():
    """创建求值器实例。"""
    return Evaluator()


class TestLambdaBasics:
    """Lambda 基础测试。"""

    def test_lambda_creates_closure(self, evaluator):
        """测试 lambda 创建闭包。"""
        result = evaluator.run("(lambda (x) x)")
        assert isinstance(result, Closure)

    def test_lambda_params(self, evaluator):
        """测试闭包的参数。"""
        result = evaluator.run("(lambda (x y) (+ x y))")
        assert result.params == ['x', 'y']

    def test_simple_lambda_call(self, evaluator):
        """测试简单的 lambda 调用。"""
        result = evaluator.run("((lambda (x) x) 42)")
        assert result == 42

    def test_lambda_with_arithmetic(self, evaluator):
        """测试带算术的 lambda。"""
        result = evaluator.run("((lambda (x) (* x x)) 5)")
        assert result == 25

    def test_lambda_multiple_params(self, evaluator):
        """测试多参数 lambda。"""
        result = evaluator.run("((lambda (x y) (+ x y)) 3 4)")
        assert result == 7


class TestDefineFunction:
    """定义函数测试。"""

    def test_define_and_call(self, evaluator):
        """测试定义并调用函数。"""
        evaluator.run("(define square (lambda (x) (* x x)))")
        result = evaluator.run("(square 5)")
        assert result == 25

    def test_define_multiple_functions(self, evaluator):
        """测试定义多个函数。"""
        evaluator.run("(define double (lambda (x) (* x 2)))")
        evaluator.run("(define triple (lambda (x) (* x 3)))")
        assert evaluator.run("(double 5)") == 10
        assert evaluator.run("(triple 5)") == 15

    def test_function_using_global(self, evaluator):
        """测试函数使用全局变量。"""
        evaluator.run("(define y 10)")
        evaluator.run("(define add-y (lambda (x) (+ x y)))")
        result = evaluator.run("(add-y 5)")
        assert result == 15


class TestClosure:
    """闭包测试 - 这是最重要的部分！"""

    def test_closure_captures_environment(self, evaluator):
        """测试闭包捕获环境。"""
        code = """
        (define make-adder
          (lambda (x)
            (lambda (y) (+ x y))))
        """
        evaluator.run(code)

        evaluator.run("(define add5 (make-adder 5))")
        result = evaluator.run("(add5 3)")
        assert result == 8

    def test_multiple_closures_independent(self, evaluator):
        """测试多个闭包相互独立。"""
        code = """
        (define make-adder
          (lambda (x)
            (lambda (y) (+ x y))))
        """
        evaluator.run(code)

        evaluator.run("(define add5 (make-adder 5))")
        evaluator.run("(define add10 (make-adder 10))")

        assert evaluator.run("(add5 1)") == 6
        assert evaluator.run("(add10 1)") == 11

    def test_closure_lexical_scope(self, evaluator):
        """测试闭包使用词法作用域。"""
        code = """
        (define x 10)
        (define f (lambda () x))
        (define g
          (lambda ()
            (define x 20)
            (f)))
        """
        evaluator.run(code)

        # 词法作用域：f 看到的是定义时的 x=10，不是调用时的 x=20
        result = evaluator.run("(g)")
        assert result == 10


class TestRecursion:
    """递归测试。"""

    def test_factorial(self, evaluator):
        """测试阶乘。"""
        code = """
        (define factorial
          (lambda (n)
            (if (= n 0)
                1
                (* n (factorial (- n 1))))))
        """
        evaluator.run(code)

        assert evaluator.run("(factorial 0)") == 1
        assert evaluator.run("(factorial 1)") == 1
        assert evaluator.run("(factorial 5)") == 120

    def test_fibonacci(self, evaluator):
        """测试斐波那契。"""
        code = """
        (define fib
          (lambda (n)
            (if (< n 2)
                n
                (+ (fib (- n 1)) (fib (- n 2))))))
        """
        evaluator.run(code)

        assert evaluator.run("(fib 0)") == 0
        assert evaluator.run("(fib 1)") == 1
        assert evaluator.run("(fib 10)") == 55


class TestHigherOrderFunctions:
    """高阶函数测试。"""

    def test_apply_twice(self, evaluator):
        """测试 apply-twice。"""
        code = """
        (define apply-twice
          (lambda (f x)
            (f (f x))))
        (define add1 (lambda (x) (+ x 1)))
        """
        evaluator.run(code)

        result = evaluator.run("(apply-twice add1 5)")
        assert result == 7

    def test_compose(self, evaluator):
        """测试函数组合。"""
        code = """
        (define compose
          (lambda (f g)
            (lambda (x) (f (g x)))))
        (define double (lambda (x) (* x 2)))
        (define add1 (lambda (x) (+ x 1)))
        (define double-then-add1 (compose add1 double))
        """
        evaluator.run(code)

        result = evaluator.run("(double-then-add1 5)")
        assert result == 11  # (5 * 2) + 1


class TestMultipleBodyExpressions:
    """多表达式函数体测试。"""

    def test_lambda_multiple_body(self, evaluator):
        """测试 lambda 有多个 body 表达式。"""
        code = """
        (define f
          (lambda (x)
            (define y (* x 2))
            (+ x y)))
        """
        evaluator.run(code)

        result = evaluator.run("(f 5)")
        assert result == 15  # 5 + 10


class TestErrors:
    """错误处理测试。"""

    def test_wrong_argument_count(self, evaluator):
        """测试参数数量错误。"""
        evaluator.run("(define f (lambda (x y) (+ x y)))")

        with pytest.raises(EvaluatorError):
            evaluator.run("(f 1)")  # 缺少参数

    def test_call_non_function(self, evaluator):
        """测试调用非函数。"""
        evaluator.run("(define x 42)")

        with pytest.raises(EvaluatorError):
            evaluator.run("(x 1)")  # 42 不是函数
