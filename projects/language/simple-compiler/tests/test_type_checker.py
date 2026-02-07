"""Tests for the type checker."""

import pytest
from src.simple_compiler.lexer import Lexer
from src.simple_compiler.parser import Parser
from src.simple_compiler.type_checker import TypeChecker
from src.simple_compiler.errors import TypeError as TypeCheckError


def type_check_code(code: str) -> None:
    """Helper function to lex, parse, and type check code."""
    lexer = Lexer(code)
    tokens = lexer.tokenize()
    parser = Parser(tokens)
    program = parser.parse()
    checker = TypeChecker()
    checker.check_program(program)


def test_type_check_simple_function():
    """Test type checking a simple function."""
    code = "int main() { return 42; }"
    type_check_code(code)  # Should not raise


def test_type_check_variable_declaration():
    """Test type checking variable declaration."""
    code = "int main() { int x = 10; return x; }"
    type_check_code(code)


def test_type_check_arithmetic():
    """Test type checking arithmetic operations."""
    code = "int main() { return 1 + 2 * 3; }"
    type_check_code(code)


def test_type_check_comparison():
    """Test type checking comparison operations."""
    code = "bool test() { return 1 < 2; }"
    type_check_code(code)


def test_type_check_logical():
    """Test type checking logical operations."""
    code = "bool test() { return true && false; }"
    type_check_code(code)


def test_type_check_function_call():
    """Test type checking function calls."""
    code = """
    int add(int a, int b) {
        return a + b;
    }
    int main() {
        return add(1, 2);
    }
    """
    type_check_code(code)


def test_type_check_if_statement():
    """Test type checking if statements."""
    code = """
    int main() {
        if (true) {
            return 1;
        } else {
            return 0;
        }
    }
    """
    type_check_code(code)


def test_type_check_while_statement():
    """Test type checking while statements."""
    code = """
    int main() {
        int x = 0;
        while (x < 10) {
            x = x + 1;
        }
        return x;
    }
    """
    type_check_code(code)


def test_type_error_wrong_return_type():
    """Test error on wrong return type."""
    code = "int main() { return true; }"
    with pytest.raises(TypeCheckError) as exc:
        type_check_code(code)
    assert "Return type mismatch" in str(exc.value)


def test_type_error_undefined_variable():
    """Test error on undefined variable."""
    code = "int main() { return x; }"
    with pytest.raises(TypeCheckError) as exc:
        type_check_code(code)
    assert "Undefined variable" in str(exc.value)


def test_type_error_type_mismatch_in_assignment():
    """Test error on type mismatch in assignment."""
    code = "int main() { int x = 0; x = true; return x; }"
    with pytest.raises(TypeCheckError) as exc:
        type_check_code(code)
    assert "Type mismatch in assignment" in str(exc.value)


def test_type_error_arithmetic_on_bool():
    """Test error on arithmetic with boolean."""
    code = "int main() { return true + 1; }"
    with pytest.raises(TypeCheckError) as exc:
        type_check_code(code)
    assert "must be int" in str(exc.value)


def test_type_error_comparison_on_bool():
    """Test error on comparison with boolean."""
    code = "bool test() { return true < false; }"
    with pytest.raises(TypeCheckError) as exc:
        type_check_code(code)
    assert "must be int" in str(exc.value)


def test_type_error_logical_on_int():
    """Test error on logical operation with integer."""
    code = "bool test() { return 1 && 2; }"
    with pytest.raises(TypeCheckError) as exc:
        type_check_code(code)
    assert "must be bool" in str(exc.value)


def test_type_error_if_condition_not_bool():
    """Test error on non-boolean if condition."""
    code = "int main() { if (42) { return 1; } return 0; }"
    with pytest.raises(TypeCheckError) as exc:
        type_check_code(code)
    assert "If condition must be boolean" in str(exc.value)


def test_type_error_while_condition_not_bool():
    """Test error on non-boolean while condition."""
    code = "int main() { while (42) { return 1; } return 0; }"
    with pytest.raises(TypeCheckError) as exc:
        type_check_code(code)
    assert "While condition must be boolean" in str(exc.value)


def test_type_error_undefined_function():
    """Test error on undefined function call."""
    code = "int main() { return foo(); }"
    with pytest.raises(TypeCheckError) as exc:
        type_check_code(code)
    assert "Undefined function" in str(exc.value)


def test_type_error_wrong_argument_count():
    """Test error on wrong number of arguments."""
    code = """
    int add(int a, int b) { return a + b; }
    int main() { return add(1); }
    """
    with pytest.raises(TypeCheckError) as exc:
        type_check_code(code)
    assert "expects 2 arguments, got 1" in str(exc.value)


def test_type_error_wrong_argument_type():
    """Test error on wrong argument type."""
    code = """
    int add(int a, int b) { return a + b; }
    int main() { return add(1, true); }
    """
    with pytest.raises(TypeCheckError) as exc:
        type_check_code(code)
    assert "Argument 2" in str(exc.value)


def test_type_check_nested_scopes():
    """Test type checking with nested scopes."""
    code = """
    int main() {
        int x = 1;
        if (true) {
            int y = 2;
            x = x + y;
        }
        return x;
    }
    """
    type_check_code(code)


def test_type_error_duplicate_variable():
    """Test error on duplicate variable declaration."""
    code = "int main() { int x = 1; int x = 2; return x; }"
    with pytest.raises(TypeCheckError) as exc:
        type_check_code(code)
    assert "already defined" in str(exc.value)


def test_type_error_duplicate_function():
    """Test error on duplicate function declaration."""
    code = """
    int foo() { return 1; }
    int foo() { return 2; }
    """
    with pytest.raises(TypeCheckError) as exc:
        type_check_code(code)
    assert "already defined" in str(exc.value)


def test_type_check_void_function():
    """Test type checking void function."""
    code = """
    void foo() {
        int x = 1;
    }
    int main() {
        foo();
        return 0;
    }
    """
    type_check_code(code)


def test_type_error_void_function_return_value():
    """Test error on void function returning value."""
    code = "void foo() { return 1; }"
    with pytest.raises(TypeCheckError) as exc:
        type_check_code(code)
    assert "Void function cannot return a value" in str(exc.value)


def test_type_check_unary_minus():
    """Test type checking unary minus."""
    code = "int main() { return -42; }"
    type_check_code(code)


def test_type_check_unary_not():
    """Test type checking unary not."""
    code = "bool test() { return !true; }"
    type_check_code(code)
