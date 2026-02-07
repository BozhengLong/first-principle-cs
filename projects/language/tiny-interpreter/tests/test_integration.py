"""Integration tests for the interpreter."""

from src.tiny_interpreter.evaluator import Evaluator


def test_factorial_example():
    """Test the factorial example."""
    evaluator = Evaluator()
    code = """
    (define factorial
      (lambda (n)
        (if (= n 0)
            1
            (* n (factorial (- n 1))))))

    (factorial 5)
    """
    result = evaluator.run(code)
    assert result == 120


def test_closure_example():
    """Test the closure example."""
    evaluator = Evaluator()
    code = """
    (define make-adder
      (lambda (x)
        (lambda (y)
          (+ x y))))

    (define add5 (make-adder 5))
    """
    evaluator.run(code)

    # Test closure captures outer variable
    assert evaluator.run("(add5 3)") == 8
    assert evaluator.run("(add5 10)") == 15


def test_higher_order_functions():
    """Test higher-order functions."""
    evaluator = Evaluator()
    code = """
    (define apply-twice
      (lambda (f x)
        (f (f x))))

    (define add1 (lambda (x) (+ x 1)))

    (apply-twice add1 5)
    """
    result = evaluator.run(code)
    assert result == 7


def test_list_processing():
    """Test list processing."""
    evaluator = Evaluator()
    code = """
    (define sum-list
      (lambda (lst)
        (if (null? lst)
            0
            (+ (car lst) (sum-list (cdr lst))))))

    (sum-list (list 1 2 3 4 5))
    """
    result = evaluator.run(code)
    assert result == 15
