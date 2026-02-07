"""Symbol table for tracking variables and functions."""

from typing import Dict, Optional
from .ast_nodes import Type
from .types import FunctionSignature


class SymbolTable:
    """Symbol table for managing variable and function scopes."""

    def __init__(self, parent: Optional['SymbolTable'] = None):
        self.parent = parent
        self.variables: Dict[str, Type] = {}
        self.functions: Dict[str, FunctionSignature] = {}

    def define_variable(self, name: str, var_type: Type) -> None:
        """Define a variable in the current scope."""
        if name in self.variables:
            raise Exception(f"Variable '{name}' already defined in this scope")
        self.variables[name] = var_type

    def lookup_variable(self, name: str) -> Optional[Type]:
        """Look up a variable in the current scope or parent scopes."""
        if name in self.variables:
            return self.variables[name]
        if self.parent:
            return self.parent.lookup_variable(name)
        return None

    def define_function(self, signature: FunctionSignature) -> None:
        """Define a function in the current scope."""
        if signature.name in self.functions:
            raise Exception(f"Function '{signature.name}' already defined")
        self.functions[signature.name] = signature

    def lookup_function(self, name: str) -> Optional[FunctionSignature]:
        """Look up a function in the current scope or parent scopes."""
        if name in self.functions:
            return self.functions[name]
        if self.parent:
            return self.parent.lookup_function(name)
        return None

    def enter_scope(self) -> 'SymbolTable':
        """Create a new nested scope."""
        return SymbolTable(parent=self)

    def exit_scope(self) -> Optional['SymbolTable']:
        """Exit the current scope and return to parent."""
        return self.parent
