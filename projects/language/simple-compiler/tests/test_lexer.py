"""Tests for the lexer."""

import pytest
from src.simple_compiler.lexer import Lexer
from src.simple_compiler.tokens import TokenType
from src.simple_compiler.errors import LexerError


def test_lex_integer():
    """Test lexing integer literals."""
    lexer = Lexer("42")
    tokens = lexer.tokenize()
    assert len(tokens) == 2  # INT_LITERAL + EOF
    assert tokens[0].type == TokenType.INT_LITERAL
    assert tokens[0].value == 42


def test_lex_boolean():
    """Test lexing boolean literals."""
    lexer = Lexer("true false")
    tokens = lexer.tokenize()
    assert len(tokens) == 3  # TRUE + FALSE + EOF
    assert tokens[0].type == TokenType.BOOL_LITERAL
    assert tokens[0].value is True
    assert tokens[1].type == TokenType.BOOL_LITERAL
    assert tokens[1].value is False


def test_lex_keywords():
    """Test lexing keywords."""
    lexer = Lexer("int bool void if else while return")
    tokens = lexer.tokenize()
    expected_types = [
        TokenType.INT, TokenType.BOOL, TokenType.VOID,
        TokenType.IF, TokenType.ELSE, TokenType.WHILE, TokenType.RETURN,
        TokenType.EOF
    ]
    assert len(tokens) == len(expected_types)
    for token, expected_type in zip(tokens, expected_types):
        assert token.type == expected_type


def test_lex_identifier():
    """Test lexing identifiers."""
    lexer = Lexer("foo bar_baz x123")
    tokens = lexer.tokenize()
    assert len(tokens) == 4  # 3 identifiers + EOF
    assert all(t.type == TokenType.IDENTIFIER for t in tokens[:-1])
    assert tokens[0].value == "foo"
    assert tokens[1].value == "bar_baz"
    assert tokens[2].value == "x123"


def test_lex_operators():
    """Test lexing arithmetic operators."""
    lexer = Lexer("+ - * / %")
    tokens = lexer.tokenize()
    expected_types = [
        TokenType.PLUS, TokenType.MINUS, TokenType.STAR,
        TokenType.SLASH, TokenType.PERCENT, TokenType.EOF
    ]
    assert len(tokens) == len(expected_types)
    for token, expected_type in zip(tokens, expected_types):
        assert token.type == expected_type


def test_lex_comparisons():
    """Test lexing comparison operators."""
    lexer = Lexer("< > <= >= == !=")
    tokens = lexer.tokenize()
    expected_types = [
        TokenType.LT, TokenType.GT, TokenType.LE,
        TokenType.GE, TokenType.EQ, TokenType.NE, TokenType.EOF
    ]
    assert len(tokens) == len(expected_types)
    for token, expected_type in zip(tokens, expected_types):
        assert token.type == expected_type


def test_lex_logical():
    """Test lexing logical operators."""
    lexer = Lexer("&& || !")
    tokens = lexer.tokenize()
    expected_types = [TokenType.AND, TokenType.OR, TokenType.NOT, TokenType.EOF]
    assert len(tokens) == len(expected_types)
    for token, expected_type in zip(tokens, expected_types):
        assert token.type == expected_type


def test_lex_delimiters():
    """Test lexing delimiters."""
    lexer = Lexer("( ) { } ; , =")
    tokens = lexer.tokenize()
    expected_types = [
        TokenType.LPAREN, TokenType.RPAREN, TokenType.LBRACE, TokenType.RBRACE,
        TokenType.SEMICOLON, TokenType.COMMA, TokenType.ASSIGN, TokenType.EOF
    ]
    assert len(tokens) == len(expected_types)
    for token, expected_type in zip(tokens, expected_types):
        assert token.type == expected_type


def test_lex_simple_program():
    """Test lexing a simple program."""
    code = """
    int main() {
        return 42;
    }
    """
    lexer = Lexer(code)
    tokens = lexer.tokenize()
    expected_types = [
        TokenType.INT, TokenType.IDENTIFIER, TokenType.LPAREN, TokenType.RPAREN,
        TokenType.LBRACE, TokenType.RETURN, TokenType.INT_LITERAL, TokenType.SEMICOLON,
        TokenType.RBRACE, TokenType.EOF
    ]
    assert len(tokens) == len(expected_types)
    for token, expected_type in zip(tokens, expected_types):
        assert token.type == expected_type


def test_lex_with_comments():
    """Test lexing with comments."""
    code = """
    // This is a comment
    int x = 10; // Another comment
    """
    lexer = Lexer(code)
    tokens = lexer.tokenize()
    # Comments should be skipped
    expected_types = [
        TokenType.INT, TokenType.IDENTIFIER, TokenType.ASSIGN,
        TokenType.INT_LITERAL, TokenType.SEMICOLON, TokenType.EOF
    ]
    assert len(tokens) == len(expected_types)
    for token, expected_type in zip(tokens, expected_types):
        assert token.type == expected_type


def test_lex_position_tracking():
    """Test that position information is tracked correctly."""
    code = "int x"
    lexer = Lexer(code)
    tokens = lexer.tokenize()
    assert tokens[0].line == 1
    assert tokens[0].column == 1
    assert tokens[1].line == 1
    assert tokens[1].column == 5


def test_lex_multiline_position():
    """Test position tracking across multiple lines."""
    code = "int\nx"
    lexer = Lexer(code)
    tokens = lexer.tokenize()
    assert tokens[0].line == 1
    assert tokens[1].line == 2
    assert tokens[1].column == 1


def test_lex_invalid_character():
    """Test that invalid characters raise an error."""
    lexer = Lexer("int x = @;")
    with pytest.raises(LexerError) as exc:
        lexer.tokenize()
    assert "Unexpected character" in str(exc.value)


def test_lex_empty_input():
    """Test lexing empty input."""
    lexer = Lexer("")
    tokens = lexer.tokenize()
    assert len(tokens) == 1
    assert tokens[0].type == TokenType.EOF


def test_lex_whitespace_only():
    """Test lexing whitespace-only input."""
    lexer = Lexer("   \n\t  ")
    tokens = lexer.tokenize()
    assert len(tokens) == 1
    assert tokens[0].type == TokenType.EOF
