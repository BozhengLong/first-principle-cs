"""Lexical analyzer for SimpleLang."""

from typing import List
from .tokens import Token, TokenType, KEYWORDS
from .errors import LexerError


class Lexer:
    """Lexer for SimpleLang source code."""

    def __init__(self, source: str):
        self.source = source
        self.pos = 0
        self.line = 1
        self.column = 1
        self.current_char = self.source[0] if source else None

    def advance(self) -> None:
        """Move to the next character."""
        if self.current_char == '\n':
            self.line += 1
            self.column = 1
        else:
            self.column += 1

        self.pos += 1
        if self.pos < len(self.source):
            self.current_char = self.source[self.pos]
        else:
            self.current_char = None

    def peek(self, offset: int = 1) -> str:
        """Look ahead at the next character without consuming it."""
        peek_pos = self.pos + offset
        if peek_pos < len(self.source):
            return self.source[peek_pos]
        return None

    def skip_whitespace(self) -> None:
        """Skip whitespace characters."""
        while self.current_char is not None and self.current_char.isspace():
            self.advance()

    def skip_comment(self) -> None:
        """Skip single-line comments starting with //."""
        if self.current_char == '/' and self.peek() == '/':
            # Skip until end of line
            while self.current_char is not None and self.current_char != '\n':
                self.advance()
            if self.current_char == '\n':
                self.advance()

    def read_number(self) -> Token:
        """Read an integer literal."""
        start_line = self.line
        start_column = self.column
        num_str = ''

        while self.current_char is not None and self.current_char.isdigit():
            num_str += self.current_char
            self.advance()

        return Token(TokenType.INT_LITERAL, int(num_str), start_line, start_column)

    def read_identifier(self) -> Token:
        """Read an identifier or keyword."""
        start_line = self.line
        start_column = self.column
        id_str = ''

        while self.current_char is not None and (self.current_char.isalnum() or self.current_char == '_'):
            id_str += self.current_char
            self.advance()

        # Check if it's a keyword
        token_type = KEYWORDS.get(id_str, TokenType.IDENTIFIER)

        # Handle boolean literals
        if token_type == TokenType.TRUE:
            return Token(TokenType.BOOL_LITERAL, True, start_line, start_column)
        elif token_type == TokenType.FALSE:
            return Token(TokenType.BOOL_LITERAL, False, start_line, start_column)

        return Token(token_type, id_str, start_line, start_column)

    def next_token(self) -> Token:
        """Get the next token from the source."""
        while self.current_char is not None:
            # Skip whitespace
            if self.current_char.isspace():
                self.skip_whitespace()
                continue

            # Skip comments
            if self.current_char == '/' and self.peek() == '/':
                self.skip_comment()
                continue

            # Numbers
            if self.current_char.isdigit():
                return self.read_number()

            # Identifiers and keywords
            if self.current_char.isalpha() or self.current_char == '_':
                return self.read_identifier()

            # Save position for token
            line = self.line
            column = self.column

            # Two-character operators
            if self.current_char == '<' and self.peek() == '=':
                self.advance()
                self.advance()
                return Token(TokenType.LE, '<=', line, column)

            if self.current_char == '>' and self.peek() == '=':
                self.advance()
                self.advance()
                return Token(TokenType.GE, '>=', line, column)

            if self.current_char == '=' and self.peek() == '=':
                self.advance()
                self.advance()
                return Token(TokenType.EQ, '==', line, column)

            if self.current_char == '!' and self.peek() == '=':
                self.advance()
                self.advance()
                return Token(TokenType.NE, '!=', line, column)

            if self.current_char == '&' and self.peek() == '&':
                self.advance()
                self.advance()
                return Token(TokenType.AND, '&&', line, column)

            if self.current_char == '|' and self.peek() == '|':
                self.advance()
                self.advance()
                return Token(TokenType.OR, '||', line, column)

            # Single-character tokens
            single_char_tokens = {
                '+': TokenType.PLUS,
                '-': TokenType.MINUS,
                '*': TokenType.STAR,
                '/': TokenType.SLASH,
                '%': TokenType.PERCENT,
                '<': TokenType.LT,
                '>': TokenType.GT,
                '!': TokenType.NOT,
                '(': TokenType.LPAREN,
                ')': TokenType.RPAREN,
                '{': TokenType.LBRACE,
                '}': TokenType.RBRACE,
                ';': TokenType.SEMICOLON,
                ',': TokenType.COMMA,
                '=': TokenType.ASSIGN,
            }

            if self.current_char in single_char_tokens:
                char = self.current_char
                token_type = single_char_tokens[char]
                self.advance()
                return Token(token_type, char, line, column)

            # Invalid character
            raise LexerError(
                f"Unexpected character: '{self.current_char}'",
                self.line,
                self.column
            )

        # End of file
        return Token(TokenType.EOF, None, self.line, self.column)

    def tokenize(self) -> List[Token]:
        """Tokenize the entire source code."""
        tokens = []
        while True:
            token = self.next_token()
            tokens.append(token)
            if token.type == TokenType.EOF:
                break
        return tokens
