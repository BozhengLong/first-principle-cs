"""Abstract Syntax Tree node definitions."""

from dataclasses import dataclass
from typing import List, Optional, Any


# Base classes
@dataclass
class ASTNode:
    """Base class for all AST nodes."""
    line: int
    column: int


# Types
@dataclass
class Type(ASTNode):
    """Base class for type nodes."""
    pass


@dataclass
class IntType(Type):
    """Integer type."""
    def __repr__(self) -> str:
        return "int"


@dataclass
class BoolType(Type):
    """Boolean type."""
    def __repr__(self) -> str:
        return "bool"


@dataclass
class VoidType(Type):
    """Void type (for functions)."""
    def __repr__(self) -> str:
        return "void"


# Expressions
@dataclass
class Expression(ASTNode):
    """Base class for expressions."""
    pass


@dataclass
class IntLiteral(Expression):
    """Integer literal expression."""
    value: int


@dataclass
class BoolLiteral(Expression):
    """Boolean literal expression."""
    value: bool


@dataclass
class Variable(Expression):
    """Variable reference expression."""
    name: str


@dataclass
class BinaryOp(Expression):
    """Binary operation expression."""
    op: str
    left: Expression
    right: Expression


@dataclass
class UnaryOp(Expression):
    """Unary operation expression."""
    op: str
    operand: Expression


@dataclass
class FunctionCall(Expression):
    """Function call expression."""
    name: str
    arguments: List[Expression]


# Statements
@dataclass
class Statement(ASTNode):
    """Base class for statements."""
    pass


@dataclass
class VarDecl(Statement):
    """Variable declaration statement."""
    var_type: Type
    name: str
    initializer: Optional[Expression] = None


@dataclass
class Assignment(Statement):
    """Assignment statement."""
    name: str
    value: Expression


@dataclass
class IfStatement(Statement):
    """If statement."""
    condition: Expression
    then_branch: List[Statement]
    else_branch: Optional[List[Statement]] = None


@dataclass
class WhileStatement(Statement):
    """While loop statement."""
    condition: Expression
    body: List[Statement]


@dataclass
class ReturnStatement(Statement):
    """Return statement."""
    value: Optional[Expression] = None


@dataclass
class ExpressionStatement(Statement):
    """Expression statement (expression followed by semicolon)."""
    expression: Expression


# Top-level declarations
@dataclass
class FunctionDecl(ASTNode):
    """Function declaration."""
    return_type: Type
    name: str
    parameters: List[tuple[Type, str]]  # List of (type, name) pairs
    body: List[Statement]


@dataclass
class Program(ASTNode):
    """Top-level program node."""
    functions: List[FunctionDecl]
