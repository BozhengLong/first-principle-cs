"""Error classes for the compiler."""


class CompilerError(Exception):
    """Base class for all compiler errors."""

    def __init__(self, message: str, line: int = 0, column: int = 0):
        self.message = message
        self.line = line
        self.column = column
        super().__init__(self.format_message())

    def format_message(self) -> str:
        """Format error message with location information."""
        if self.line > 0:
            return f"Line {self.line}, Column {self.column}: {self.message}"
        return self.message


class LexerError(CompilerError):
    """Error during lexical analysis."""
    pass


class ParserError(CompilerError):
    """Error during parsing."""
    pass


class TypeError(CompilerError):
    """Error during type checking."""
    pass


class CodeGenError(CompilerError):
    """Error during code generation."""
    pass


class RuntimeError(CompilerError):
    """Error during runtime execution."""
    pass
