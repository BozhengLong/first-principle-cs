"""Integration tests for the complete compiler."""

import pytest
from src.simple_compiler.main import compile_and_run


def test_simple_return():
    """Test simple return statement."""
    code = "int main() { return 42; }"
    result = compile_and_run(code)
    assert result == 42


def test_arithmetic():
    """Test arithmetic operations."""
    code = """
    int main() {
        int x = 10;
        int y = 20;
        return x + y;
    }
    """
    result = compile_and_run(code)
    assert result == 30


def test_multiplication():
    """Test multiplication."""
    code = """
    int main() {
        int x = 5;
        int y = 6;
        return x * y;
    }
    """
    result = compile_and_run(code)
    assert result == 30


def test_subtraction():
    """Test subtraction."""
    code = """
    int main() {
        int x = 50;
        int y = 8;
        return x - y;
    }
    """
    result = compile_and_run(code)
    assert result == 42


def test_division():
    """Test division."""
    code = """
    int main() {
        int x = 84;
        int y = 2;
        return x / y;
    }
    """
    result = compile_and_run(code)
    assert result == 42


def test_modulo():
    """Test modulo operation."""
    code = """
    int main() {
        int x = 17;
        int y = 5;
        return x % y;
    }
    """
    result = compile_and_run(code)
    assert result == 2


def test_comparison():
    """Test comparison operations."""
    code = """
    int main() {
        int x = 10;
        int y = 20;
        if (x < y) {
            return 1;
        } else {
            return 0;
        }
    }
    """
    result = compile_and_run(code)
    assert result == 1


def test_if_else():
    """Test if-else statement."""
    code = """
    int main() {
        int x = 5;
        if (x > 10) {
            return 0;
        } else {
            return 42;
        }
    }
    """
    result = compile_and_run(code)
    assert result == 42


def test_while_loop():
    """Test while loop."""
    code = """
    int main() {
        int x = 0;
        int sum = 0;
        while (x < 10) {
            sum = sum + x;
            x = x + 1;
        }
        return sum;
    }
    """
    result = compile_and_run(code)
    assert result == 45  # 0+1+2+...+9 = 45


def test_nested_if():
    """Test nested if statements."""
    code = """
    int main() {
        int x = 15;
        if (x > 10) {
            if (x < 20) {
                return 1;
            } else {
                return 2;
            }
        } else {
            return 3;
        }
    }
    """
    result = compile_and_run(code)
    assert result == 1


def test_complex_expression():
    """Test complex arithmetic expression."""
    code = """
    int main() {
        return 2 + 3 * 4 - 1;
    }
    """
    result = compile_and_run(code)
    assert result == 13  # 2 + 12 - 1 = 13


def test_boolean_operations():
    """Test boolean operations."""
    code = """
    int main() {
        if (true && true) {
            return 1;
        } else {
            return 0;
        }
    }
    """
    result = compile_and_run(code)
    assert result == 1


def test_logical_or():
    """Test logical OR."""
    code = """
    int main() {
        if (false || true) {
            return 1;
        } else {
            return 0;
        }
    }
    """
    result = compile_and_run(code)
    assert result == 1


def test_logical_not():
    """Test logical NOT."""
    code = """
    int main() {
        if (!false) {
            return 1;
        } else {
            return 0;
        }
    }
    """
    result = compile_and_run(code)
    assert result == 1


def test_unary_minus():
    """Test unary minus."""
    code = """
    int main() {
        int x = 10;
        return -x;
    }
    """
    result = compile_and_run(code)
    assert result == -10
