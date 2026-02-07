"""Main compiler entry point."""

import sys
from .lexer import Lexer
from .parser import Parser
from .type_checker import TypeChecker
from .ir_generator import IRGenerator
from .code_generator import CodeGenerator
from .vm import VM


def compile_and_run(source_code: str) -> int:
    """Compile and run SimpleLang source code."""
    # Lexical analysis
    lexer = Lexer(source_code)
    tokens = lexer.tokenize()

    # Parsing
    parser = Parser(tokens)
    program = parser.parse()

    # Type checking
    type_checker = TypeChecker()
    type_checker.check_program(program)

    # IR generation
    ir_generator = IRGenerator()
    ir_instructions = ir_generator.generate(program)

    # Code generation
    code_generator = CodeGenerator()
    bytecode = code_generator.generate(ir_instructions)

    # Execution
    vm = VM(bytecode)
    result = vm.execute()

    return result


def main():
    """Main entry point for the compiler."""
    if len(sys.argv) < 2:
        print("Usage: python -m src.simple_compiler.main <source_file>")
        sys.exit(1)

    source_file = sys.argv[1]

    try:
        with open(source_file, 'r') as f:
            source_code = f.read()

        result = compile_and_run(source_code)
        print(f"Program exited with code: {result}")

    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main()
