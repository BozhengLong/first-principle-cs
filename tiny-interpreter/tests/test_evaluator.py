"""Tests for the evaluator."""

import pytest
from src.tiny_interpreter.evaluator import Evaluator, EvaluatorError


def test_eval_number():
    """Test evaluating a number."""
    evaluator = Evaluator()
    result = evaluator.run("42")
    assert result == 42


def test_eval_boolean():
    """Test evaluating a boolean."""
    evaluator = Evaluator()
    assert evaluator.run("#t") is True
    assert evaluator.run("#f") is False


def test_eval_arithmetic():
    """Test evaluating arithmetic expressions."""
    evaluator = Evaluator()
    assert evaluator.run("(+ 1 2)") == 3
    assert evaluator.run("(- 5 3)") == 2
    assert evaluator.run("(* 3 4)") == 12
    assert evaluator.run("(/ 10 2)") == 5


def test_eval_nested_arithmetic():
    """Test evaluating nested arithmetic."""
    evaluator = Evaluator()
    assert evaluator.run("(+ (* 2 3) 4)") == 10
    assert evaluator.run("(* (+ 1 2) (- 5 3))") == 6


def test_eval_comparison():
    """Test evaluating comparison operations."""
    evaluator = Evaluator()
    assert evaluator.run("(= 1 1)") is True
    assert evaluator.run("(= 1 2)") is False
    assert evaluator.run("(< 1 2)") is True
    assert evaluator.run("(> 2 1)") is True


def test_eval_define():
    """Test evaluating define."""
    evaluator = Evaluator()
    evaluator.run("(define x 42)")
    result = evaluator.run("x")
    assert result == 42


def test_eval_lambda():
    """Test evaluating lambda."""
    evaluator = Evaluator()
    evaluator.run("(define square (lambda (x) (* x x)))")
    result = evaluator.run("(square 5)")
    assert result == 25


def test_eval_if():
    """Test evaluating if expressions."""
    evaluator = Evaluator()
    assert evaluator.run("(if #t 1 2)") == 1
    assert evaluator.run("(if #f 1 2)") == 2
    assert evaluator.run("(if (< 1 2) 10 20)") == 10


def test_eval_closure():
    """Test evaluating closures."""
    evaluator = Evaluator()
    evaluator.run("""
        (define make-adder
          (lambda (x)
            (lambda (y)
              (+ x y))))
    """)
    evaluator.run("(define add5 (make-adder 5))")
    result = evaluator.run("(add5 3)")
    assert result == 8


def test_eval_recursion():
    """Test evaluating recursive functions."""
    evaluator = Evaluator()
    evaluator.run("""
        (define factorial
          (lambda (n)
            (if (= n 0)
                1
                (* n (factorial (- n 1))))))
    """)
    assert evaluator.run("(factorial 5)") == 120


def test_eval_list_operations():
    """Test evaluating list operations."""
    evaluator = Evaluator()
    evaluator.run("(define lst (list 1 2 3))")
    assert evaluator.run("(car lst)") == 1
    assert evaluator.run("(car (cdr lst))") == 2


def test_eval_quote():
    """Test evaluating quote."""
    evaluator = Evaluator()
    result = evaluator.run("(quote (1 2 3))")
    assert result == [1, 2, 3]


def test_eval_undefined_variable():
    """Test evaluating undefined variable."""
    evaluator = Evaluator()
    with pytest.raises(NameError):
        evaluator.run("undefined-var")


def test_eval_wrong_number_of_args():
    """Test calling function with wrong number of arguments."""
    evaluator = Evaluator()
    evaluator.run("(define f (lambda (x) x))")
    with pytest.raises(EvaluatorError):
        evaluator.run("(f 1 2)")


def test_eval_multiple_expressions():
    """Test evaluating multiple expressions."""
    evaluator = Evaluator()
    result = evaluator.run("""
        (define x 1)
        (define y 2)
        (+ x y)
    """)
    assert result == 3
