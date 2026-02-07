"""Tests for the lexer."""

import pytest
from src.tiny_interpreter.lexer import Lexer, TokenType, LexerError


def test_empty_input():
    """Test lexing empty input."""
    lexer = Lexer("")
    tokens = lexer.tokenize()
    assert len(tokens) == 1
    assert tokens[0].type == TokenType.EOF


def test_whitespace_only():
    """Test lexing whitespace only."""
    lexer = Lexer("   \n\t  ")
    tokens = lexer.tokenize()
    assert len(tokens) == 1
    assert tokens[0].type == TokenType.EOF


def test_single_number():
    """Test lexing a single number."""
    lexer = Lexer("42")
    tokens = lexer.tokenize()
    assert len(tokens) == 2  # NUMBER + EOF
    assert tokens[0].type == TokenType.NUMBER
    assert tokens[0].value == 42


def test_negative_number():
    """Test lexing a negative number."""
    lexer = Lexer("-42")
    tokens = lexer.tokenize()
    assert len(tokens) == 2
    assert tokens[0].type == TokenType.NUMBER
    assert tokens[0].value == -42


def test_single_symbol():
    """Test lexing a single symbol."""
    lexer = Lexer("foo")
    tokens = lexer.tokenize()
    assert len(tokens) == 2  # SYMBOL + EOF
    assert tokens[0].type == TokenType.SYMBOL
    assert tokens[0].value == "foo"


def test_boolean_true():
    """Test lexing boolean true."""
    lexer = Lexer("#t")
    tokens = lexer.tokenize()
    assert len(tokens) == 2
    assert tokens[0].type == TokenType.BOOLEAN
    assert tokens[0].value is True


def test_boolean_false():
    """Test lexing boolean false."""
    lexer = Lexer("#f")
    tokens = lexer.tokenize()
    assert len(tokens) == 2
    assert tokens[0].type == TokenType.BOOLEAN
    assert tokens[0].value is False


def test_parentheses():
    """Test lexing parentheses."""
    lexer = Lexer("()")
    tokens = lexer.tokenize()
    assert len(tokens) == 3  # LPAREN + RPAREN + EOF
    assert tokens[0].type == TokenType.LPAREN
    assert tokens[1].type == TokenType.RPAREN


def test_simple_expression():
    """Test lexing a simple expression."""
    lexer = Lexer("(+ 1 2)")
    tokens = lexer.tokenize()
    assert len(tokens) == 6  # LPAREN + SYMBOL + NUMBER + NUMBER + RPAREN + EOF
    assert tokens[0].type == TokenType.LPAREN
    assert tokens[1].type == TokenType.SYMBOL
    assert tokens[1].value == "+"
    assert tokens[2].type == TokenType.NUMBER
    assert tokens[2].value == 1
    assert tokens[3].type == TokenType.NUMBER
    assert tokens[3].value == 2
    assert tokens[4].type == TokenType.RPAREN


def test_nested_expression():
    """Test lexing a nested expression."""
    lexer = Lexer("(+ (* 2 3) 4)")
    tokens = lexer.tokenize()
    assert tokens[0].type == TokenType.LPAREN
    assert tokens[1].type == TokenType.SYMBOL
    assert tokens[2].type == TokenType.LPAREN
    assert tokens[3].type == TokenType.SYMBOL
    assert tokens[4].type == TokenType.NUMBER
    assert tokens[5].type == TokenType.NUMBER
    assert tokens[6].type == TokenType.RPAREN
    assert tokens[7].type == TokenType.NUMBER
    assert tokens[8].type == TokenType.RPAREN


def test_comment():
    """Test lexing with comments."""
    lexer = Lexer("; This is a comment\n42")
    tokens = lexer.tokenize()
    assert len(tokens) == 2  # NUMBER + EOF
    assert tokens[0].type == TokenType.NUMBER
    assert tokens[0].value == 42


def test_multiple_expressions():
    """Test lexing multiple expressions."""
    lexer = Lexer("1 2 3")
    tokens = lexer.tokenize()
    assert len(tokens) == 4  # NUMBER + NUMBER + NUMBER + EOF
    assert all(t.type == TokenType.NUMBER for t in tokens[:-1])


def test_symbol_with_special_chars():
    """Test lexing symbols with special characters."""
    lexer = Lexer("foo-bar? baz!")
    tokens = lexer.tokenize()
    assert tokens[0].type == TokenType.SYMBOL
    assert tokens[0].value == "foo-bar?"
    assert tokens[1].type == TokenType.SYMBOL
    assert tokens[1].value == "baz!"


def test_invalid_boolean():
    """Test lexing invalid boolean."""
    lexer = Lexer("#x")
    with pytest.raises(LexerError):
        lexer.tokenize()


def test_position_tracking():
    """Test that position information is tracked correctly."""
    lexer = Lexer("(+ 1\n  2)")
    tokens = lexer.tokenize()
    assert tokens[0].line == 1
    assert tokens[0].column == 1
    assert tokens[2].line == 1
    assert tokens[3].line == 2
