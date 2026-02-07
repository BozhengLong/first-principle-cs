"""Type definitions for the type checker."""

from dataclasses import dataclass
from typing import List, Optional
from .ast_nodes import Type, IntType, BoolType, VoidType


@dataclass
class FunctionSignature:
    """Function signature for type checking."""
    name: str
    return_type: Type
    parameter_types: List[Type]
    parameter_names: List[str]

    def __repr__(self) -> str:
        params = ', '.join(f"{t} {n}" for t, n in zip(self.parameter_types, self.parameter_names))
        return f"{self.return_type} {self.name}({params})"


def types_equal(t1: Type, t2: Type) -> bool:
    """Check if two types are equal."""
    return type(t1) == type(t2)


def type_to_string(t: Type) -> str:
    """Convert type to string representation."""
    if isinstance(t, IntType):
        return "int"
    elif isinstance(t, BoolType):
        return "bool"
    elif isinstance(t, VoidType):
        return "void"
    else:
        return str(t)
