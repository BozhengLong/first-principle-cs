/**
 * Reference implementations for cross-module dependencies.
 * These are written to Pyodide's virtual FS so skeleton files can
 * `from src.tiny_interpreter.X import ...` correctly.
 */

export const refInit = `"""Tiny Interpreter - A minimal Lisp-style interpreter."""

__version__ = "0.1.0"
`;

export const refLexer = `"""Lexer for Tiny Interpreter.

The lexer converts a string of source code into a sequence of tokens.
"""

from dataclasses import dataclass
from enum import Enum, auto
from typing import List, Optional


class TokenType(Enum):
    """Token types for the Tiny Interpreter."""
    LPAREN = auto()      # (
    RPAREN = auto()      # )
    NUMBER = auto()      # 123
    SYMBOL = auto()      # foo
    BOOLEAN = auto()     # #t or #f
    EOF = auto()         # End of file


@dataclass
class Token:
    """A token with type, value, and position information."""
    type: TokenType
    value: any
    line: int
    column: int

    def __repr__(self):
        return f"Token({self.type.name}, {self.value!r}, {self.line}:{self.column})"


class LexerError(Exception):
    """Exception raised for lexer errors."""
    def __init__(self, message: str, line: int, column: int):
        self.message = message
        self.line = line
        self.column = column
        super().__init__(f"{message} at line {line}, column {column}")


class Lexer:
    """Lexer for tokenizing Lisp-style source code."""

    def __init__(self, source: str):
        self.source = source
        self.pos = 0
        self.line = 1
        self.column = 1

    def current_char(self) -> Optional[str]:
        if self.pos >= len(self.source):
            return None
        return self.source[self.pos]

    def peek_char(self, offset: int = 1) -> Optional[str]:
        pos = self.pos + offset
        if pos >= len(self.source):
            return None
        return self.source[pos]

    def advance(self) -> Optional[str]:
        char = self.current_char()
        if char is not None:
            self.pos += 1
            if char == '\\n':
                self.line += 1
                self.column = 1
            else:
                self.column += 1
        return char

    def skip_whitespace(self):
        while self.current_char() and self.current_char().isspace():
            self.advance()

    def skip_comment(self):
        if self.current_char() == ';':
            while self.current_char() and self.current_char() != '\\n':
                self.advance()

    def read_number(self) -> Token:
        start_line = self.line
        start_column = self.column
        num_str = ''
        if self.current_char() == '-':
            num_str += self.advance()
        while self.current_char() and self.current_char().isdigit():
            num_str += self.advance()
        return Token(TokenType.NUMBER, int(num_str), start_line, start_column)

    def read_symbol(self) -> Token:
        start_line = self.line
        start_column = self.column
        symbol = ''
        while self.current_char() and self.is_symbol_char(self.current_char()):
            symbol += self.advance()
        return Token(TokenType.SYMBOL, symbol, start_line, start_column)

    def read_boolean(self) -> Token:
        start_line = self.line
        start_column = self.column
        self.advance()  # Skip #
        char = self.current_char()
        if char == 't':
            self.advance()
            return Token(TokenType.BOOLEAN, True, start_line, start_column)
        elif char == 'f':
            self.advance()
            return Token(TokenType.BOOLEAN, False, start_line, start_column)
        else:
            raise LexerError(f"Invalid boolean: #{char}", start_line, start_column)

    def is_symbol_char(self, char: str) -> bool:
        return (char.isalnum() or char in '+-*/=<>!?_')

    def next_token(self) -> Token:
        self.skip_whitespace()
        while self.current_char() == ';':
            self.skip_comment()
            self.skip_whitespace()
        char = self.current_char()
        if char is None:
            return Token(TokenType.EOF, None, self.line, self.column)
        if char == '(':
            token = Token(TokenType.LPAREN, '(', self.line, self.column)
            self.advance()
            return token
        if char == ')':
            token = Token(TokenType.RPAREN, ')', self.line, self.column)
            self.advance()
            return token
        if char == '#':
            return self.read_boolean()
        if char.isdigit() or (char == '-' and self.peek_char() and self.peek_char().isdigit()):
            return self.read_number()
        if self.is_symbol_char(char):
            return self.read_symbol()
        raise LexerError(f"Unexpected character: {char!r}", self.line, self.column)

    def tokenize(self) -> List[Token]:
        tokens = []
        while True:
            token = self.next_token()
            tokens.append(token)
            if token.type == TokenType.EOF:
                break
        return tokens
`;

export const refParser = `"""Parser for Tiny Interpreter.

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
        if self.pos >= len(self.tokens):
            return self.tokens[-1]
        return self.tokens[self.pos]

    def advance(self) -> Token:
        token = self.current_token()
        if self.pos < len(self.tokens) - 1:
            self.pos += 1
        return token

    def expect(self, token_type: TokenType) -> Token:
        token = self.current_token()
        if token.type != token_type:
            raise ParserError(
                f"Expected {token_type.name}, got {token.type.name}",
                token.line,
                token.column
            )
        return self.advance()

    def parse_atom(self) -> ASTNode:
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
        token = self.current_token()
        if token.type == TokenType.LPAREN:
            return self.parse_sexp()
        else:
            return self.parse_atom()

    def parse(self) -> List[ASTNode]:
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
`;

export const refEnvironment = `"""Environment model for Tiny Interpreter.

The environment manages variable bindings and supports lexical scoping.
"""

from typing import Dict, Optional, Any


class Environment:
    """Environment for storing variable bindings.

    Supports lexical scoping through parent environment references.
    """

    def __init__(self, parent: Optional['Environment'] = None):
        self.bindings: Dict[str, Any] = {}
        self.parent = parent

    def define(self, name: str, value: Any):
        self.bindings[name] = value

    def get(self, name: str) -> Any:
        if name in self.bindings:
            return self.bindings[name]
        if self.parent is not None:
            return self.parent.get(name)
        raise NameError(f"Undefined variable: {name}")

    def set(self, name: str, value: Any):
        if name in self.bindings:
            self.bindings[name] = value
            return
        if self.parent is not None:
            self.parent.set(name, value)
            return
        raise NameError(f"Undefined variable: {name}")

    def __repr__(self):
        return f"Environment({list(self.bindings.keys())})"
`;

export const refSrcInit = ``;
