"""Parser for Tiny Interpreter.

The parser converts a sequence of tokens into an Abstract Syntax Tree (AST).
"""

from dataclasses import dataclass
from typing import List, Union
from .lexer import Token, TokenType, Lexer


# AST Node Types
@dataclass
class Number:
    """AST node for numbers."""
    value: int
    line: int
    column: int

    def __repr__(self):
        return f"Number({self.value})"


@dataclass
class Boolean:
    """AST node for booleans."""
    value: bool
    line: int
    column: int

    def __repr__(self):
        return f"Boolean({self.value})"


@dataclass
class Symbol:
    """AST node for symbols."""
    name: str
    line: int
    column: int

    def __repr__(self):
        return f"Symbol({self.name!r})"


@dataclass
class SExpression:
    """AST node for S-expressions (lists)."""
    elements: List['ASTNode']
    line: int
    column: int

    def __repr__(self):
        return f"SExpression({self.elements})"


# Type alias for any AST node
ASTNode = Union[Number, Boolean, Symbol, SExpression]


class ParserError(Exception):
    """Exception raised for parser errors."""
    def __init__(self, message: str, line: int, column: int):
        self.message = message
        self.line = line
        self.column = column
        super().__init__(f"{message} at line {line}, column {column}")


class Parser:
    """Parser for converting tokens to AST."""

    def __init__(self, tokens: List[Token]):
        self.tokens = tokens
        self.pos = 0

    def current_token(self) -> Token:
        """Return the current token."""
        if self.pos >= len(self.tokens):
            return self.tokens[-1]  # Return EOF token
        return self.tokens[self.pos]

    def advance(self) -> Token:
        """Consume and return the current token."""
        token = self.current_token()
        if self.pos < len(self.tokens) - 1:
            self.pos += 1
        return token

    def expect(self, token_type: TokenType) -> Token:
        """Expect a specific token type and consume it."""
        token = self.current_token()
        if token.type != token_type:
            raise ParserError(
                f"Expected {token_type.name}, got {token.type.name}",
                token.line,
                token.column
            )
        return self.advance()

    def parse_atom(self) -> ASTNode:
        """Parse an atomic expression (number, boolean, or symbol)."""
        token = self.current_token()

        if token.type == TokenType.NUMBER:
            self.advance()
            return Number(token.value, token.line, token.column)

        if token.type == TokenType.BOOLEAN:
            self.advance()
            return Boolean(token.value, token.line, token.column)

        if token.type == TokenType.SYMBOL:
            self.advance()
            return Symbol(token.value, token.line, token.column)

        raise ParserError(
            f"Unexpected token: {token.type.name}",
            token.line,
            token.column
        )

    def parse_sexp(self) -> SExpression:
        """Parse an S-expression (list)."""
        lparen = self.expect(TokenType.LPAREN)
        elements = []

        while self.current_token().type != TokenType.RPAREN:
            if self.current_token().type == TokenType.EOF:
                raise ParserError(
                    "Unexpected EOF, expected ')'",
                    self.current_token().line,
                    self.current_token().column
                )
            elements.append(self.parse_expr())

        self.expect(TokenType.RPAREN)
        return SExpression(elements, lparen.line, lparen.column)

    def parse_expr(self) -> ASTNode:
        """Parse an expression."""
        token = self.current_token()

        if token.type == TokenType.LPAREN:
            return self.parse_sexp()
        else:
            return self.parse_atom()

    def parse(self) -> List[ASTNode]:
        """Parse all expressions in the token stream."""
        expressions = []

        while self.current_token().type != TokenType.EOF:
            expressions.append(self.parse_expr())

        return expressions


def parse(source: str) -> List[ASTNode]:
    """Convenience function to lex and parse source code."""
    lexer = Lexer(source)
    tokens = lexer.tokenize()
    parser = Parser(tokens)
    return parser.parse()
