"""Lexer for Tiny Interpreter.

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
        """Return the current character, or None if at end."""
        if self.pos >= len(self.source):
            return None
        return self.source[self.pos]

    def peek_char(self, offset: int = 1) -> Optional[str]:
        """Peek ahead at a character without consuming it."""
        pos = self.pos + offset
        if pos >= len(self.source):
            return None
        return self.source[pos]

    def advance(self) -> Optional[str]:
        """Consume and return the current character."""
        char = self.current_char()
        if char is not None:
            self.pos += 1
            if char == '\n':
                self.line += 1
                self.column = 1
            else:
                self.column += 1
        return char

    def skip_whitespace(self):
        """Skip whitespace characters."""
        while self.current_char() and self.current_char().isspace():
            self.advance()

    def skip_comment(self):
        """Skip a comment (from ; to end of line)."""
        if self.current_char() == ';':
            while self.current_char() and self.current_char() != '\n':
                self.advance()

    def read_number(self) -> Token:
        """Read a number token."""
        start_line = self.line
        start_column = self.column
        num_str = ''

        # Handle negative numbers
        if self.current_char() == '-':
            num_str += self.advance()

        while self.current_char() and self.current_char().isdigit():
            num_str += self.advance()

        return Token(TokenType.NUMBER, int(num_str), start_line, start_column)

    def read_symbol(self) -> Token:
        """Read a symbol token."""
        start_line = self.line
        start_column = self.column
        symbol = ''

        while self.current_char() and self.is_symbol_char(self.current_char()):
            symbol += self.advance()

        return Token(TokenType.SYMBOL, symbol, start_line, start_column)

    def read_boolean(self) -> Token:
        """Read a boolean token (#t or #f)."""
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
        """Check if a character can be part of a symbol."""
        return (char.isalnum() or
                char in '+-*/=<>!?_')

    def next_token(self) -> Token:
        """Read and return the next token."""
        self.skip_whitespace()

        # Skip comments
        while self.current_char() == ';':
            self.skip_comment()
            self.skip_whitespace()

        char = self.current_char()

        if char is None:
            return Token(TokenType.EOF, None, self.line, self.column)

        # Parentheses
        if char == '(':
            token = Token(TokenType.LPAREN, '(', self.line, self.column)
            self.advance()
            return token

        if char == ')':
            token = Token(TokenType.RPAREN, ')', self.line, self.column)
            self.advance()
            return token

        # Boolean
        if char == '#':
            return self.read_boolean()

        # Number (including negative)
        if char.isdigit() or (char == '-' and self.peek_char() and self.peek_char().isdigit()):
            return self.read_number()

        # Symbol
        if self.is_symbol_char(char):
            return self.read_symbol()

        raise LexerError(f"Unexpected character: {char!r}", self.line, self.column)

    def tokenize(self) -> List[Token]:
        """Tokenize the entire source code."""
        tokens = []
        while True:
            token = self.next_token()
            tokens.append(token)
            if token.type == TokenType.EOF:
                break
        return tokens
