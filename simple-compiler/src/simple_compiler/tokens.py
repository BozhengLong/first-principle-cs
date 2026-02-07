"""Token definitions for the lexer."""

from enum import Enum, auto
from dataclasses import dataclass
from typing import Any


class TokenType(Enum):
    """Token types for SimpleLang."""

    # Literals
    INT_LITERAL = auto()
    BOOL_LITERAL = auto()

    # Identifiers and Keywords
    IDENTIFIER = auto()
    INT = auto()
    BOOL = auto()
    VOID = auto()
    IF = auto()
    ELSE = auto()
    WHILE = auto()
    RETURN = auto()
    TRUE = auto()
    FALSE = auto()

    # Operators
    PLUS = auto()          # +
    MINUS = auto()         # -
    STAR = auto()          # *
    SLASH = auto()         # /
    PERCENT = auto()       # %

    # Comparisons
    LT = auto()            # <
    GT = auto()            # >
    LE = auto()            # <=
    GE = auto()            # >=
    EQ = auto()            # ==
    NE = auto()            # !=

    # Logical
    AND = auto()           # &&
    OR = auto()            # ||
    NOT = auto()           # !

    # Delimiters
    LPAREN = auto()        # (
    RPAREN = auto()        # )
    LBRACE = auto()        # {
    RBRACE = auto()        # }
    SEMICOLON = auto()     # ;
    COMMA = auto()         # ,
    ASSIGN = auto()        # =

    EOF = auto()


@dataclass
class Token:
    """A token with type, value, and position information."""

    type: TokenType
    value: Any
    line: int
    column: int

    def __repr__(self) -> str:
        return f"Token({self.type.name}, {self.value!r}, {self.line}:{self.column})"


# Keywords mapping
KEYWORDS = {
    'int': TokenType.INT,
    'bool': TokenType.BOOL,
    'void': TokenType.VOID,
    'if': TokenType.IF,
    'else': TokenType.ELSE,
    'while': TokenType.WHILE,
    'return': TokenType.RETURN,
    'true': TokenType.TRUE,
    'false': TokenType.FALSE,
}
