"""Tests for the parser."""

import pytest
from src.tiny_interpreter.parser import parse, Number, Boolean, Symbol, SExpression, ParserError


def test_parse_number():
    """Test parsing a number."""
    ast = parse("42")
    assert len(ast) == 1
    assert isinstance(ast[0], Number)
    assert ast[0].value == 42


def test_parse_boolean():
    """Test parsing a boolean."""
    ast = parse("#t")
    assert len(ast) == 1
    assert isinstance(ast[0], Boolean)
    assert ast[0].value is True


def test_parse_symbol():
    """Test parsing a symbol."""
    ast = parse("foo")
    assert len(ast) == 1
    assert isinstance(ast[0], Symbol)
    assert ast[0].name == "foo"


def test_parse_empty_list():
    """Test parsing an empty list."""
    ast = parse("()")
    assert len(ast) == 1
    assert isinstance(ast[0], SExpression)
    assert len(ast[0].elements) == 0


def test_parse_simple_list():
    """Test parsing a simple list."""
    ast = parse("(+ 1 2)")
    assert len(ast) == 1
    assert isinstance(ast[0], SExpression)
    assert len(ast[0].elements) == 3
    assert isinstance(ast[0].elements[0], Symbol)
    assert ast[0].elements[0].name == "+"
    assert isinstance(ast[0].elements[1], Number)
    assert ast[0].elements[1].value == 1


def test_parse_nested_list():
    """Test parsing a nested list."""
    ast = parse("(+ (* 2 3) 4)")
    assert len(ast) == 1
    assert isinstance(ast[0], SExpression)
    assert isinstance(ast[0].elements[1], SExpression)


def test_parse_multiple_expressions():
    """Test parsing multiple expressions."""
    ast = parse("1 2 3")
    assert len(ast) == 3
    assert all(isinstance(node, Number) for node in ast)


def test_parse_unmatched_paren():
    """Test parsing with unmatched parenthesis."""
    with pytest.raises(ParserError):
        parse("(+ 1 2")


def test_parse_extra_paren():
    """Test parsing with extra closing parenthesis."""
    # Extra closing paren should cause an error
    with pytest.raises(ParserError):
        parse("(+ 1 2))")
