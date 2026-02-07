"""Environment model for Tiny Interpreter.

The environment manages variable bindings and supports lexical scoping.
"""

from typing import Dict, Optional, Any


class Environment:
    """Environment for storing variable bindings.

    Supports lexical scoping through parent environment references.
    """

    def __init__(self, parent: Optional['Environment'] = None):
        """Create a new environment.

        Args:
            parent: Parent environment for lexical scoping.
        """
        self.bindings: Dict[str, Any] = {}
        self.parent = parent

    def define(self, name: str, value: Any):
        """Define a new variable in this environment.

        Args:
            name: Variable name.
            value: Variable value.
        """
        self.bindings[name] = value

    def get(self, name: str) -> Any:
        """Look up a variable value.

        Args:
            name: Variable name.

        Returns:
            The variable value.

        Raises:
            NameError: If the variable is not defined.
        """
        if name in self.bindings:
            return self.bindings[name]

        if self.parent is not None:
            return self.parent.get(name)

        raise NameError(f"Undefined variable: {name}")

    def set(self, name: str, value: Any):
        """Set an existing variable's value.

        Args:
            name: Variable name.
            value: New value.

        Raises:
            NameError: If the variable is not defined.
        """
        if name in self.bindings:
            self.bindings[name] = value
            return

        if self.parent is not None:
            self.parent.set(name, value)
            return

        raise NameError(f"Undefined variable: {name}")

    def __repr__(self):
        return f"Environment({list(self.bindings.keys())})"
