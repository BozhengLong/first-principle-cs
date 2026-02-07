"""Tests for the parser."""

import pytest
from src.simple_compiler.lexer import Lexer
from src.simple_compiler.parser import Parser
from src.simple_compiler.ast_nodes import *
from src.simple_compiler.errors import ParserError


def parse_code(code: str) -> Program:
    """Helper function to lex and parse code."""
    lexer = Lexer(code)
    tokens = lexer.tokenize()
    parser = Parser(tokens)
    return parser.parse()


def test_parse_integer_literal():
    """Test parsing integer literal."""
    code = "int main() { return 42; }"
    program = parse_code(code)
    assert len(program.functions) == 1
    func = program.functions[0]
    assert isinstance(func.body[0], ReturnStatement)
    assert isinstance(func.body[0].value, IntLiteral)
    assert func.body[0].value.value == 42


def test_parse_boolean_literal():
    """Test parsing boolean literal."""
    code = "bool test() { return true; }"
    program = parse_code(code)
    func = program.functions[0]
    assert isinstance(func.body[0].value, BoolLiteral)
    assert func.body[0].value.value is True


def test_parse_variable():
    """Test parsing variable reference."""
    code = "int test() { int x = 10; return x; }"
    program = parse_code(code)
    func = program.functions[0]
    assert isinstance(func.body[1].value, Variable)
    assert func.body[1].value.name == "x"


def test_parse_binary_op():
    """Test parsing binary operation."""
    code = "int main() { return 1 + 2; }"
    program = parse_code(code)
    func = program.functions[0]
    ret_stmt = func.body[0]
    assert isinstance(ret_stmt.value, BinaryOp)
    assert ret_stmt.value.op == '+'
    assert isinstance(ret_stmt.value.left, IntLiteral)
    assert isinstance(ret_stmt.value.right, IntLiteral)


def test_parse_expression_precedence():
    """Test expression precedence."""
    code = "int main() { return 1 + 2 * 3; }"
    program = parse_code(code)
    func = program.functions[0]
    expr = func.body[0].value
    # Should parse as 1 + (2 * 3)
    assert isinstance(expr, BinaryOp)
    assert expr.op == '+'
    assert isinstance(expr.left, IntLiteral)
    assert isinstance(expr.right, BinaryOp)
    assert expr.right.op == '*'


def test_parse_unary_op():
    """Test parsing unary operation."""
    code = "int main() { return -5; }"
    program = parse_code(code)
    func = program.functions[0]
    expr = func.body[0].value
    assert isinstance(expr, UnaryOp)
    assert expr.op == '-'
    assert isinstance(expr.operand, IntLiteral)


def test_parse_function_call():
    """Test parsing function call."""
    code = "int main() { return foo(1, 2); }"
    program = parse_code(code)
    func = program.functions[0]
    expr = func.body[0].value
    assert isinstance(expr, FunctionCall)
    assert expr.name == "foo"
    assert len(expr.arguments) == 2


def test_parse_var_decl():
    """Test parsing variable declaration."""
    code = "int main() { int x = 10; return x; }"
    program = parse_code(code)
    func = program.functions[0]
    assert isinstance(func.body[0], VarDecl)
    assert func.body[0].name == "x"
    assert isinstance(func.body[0].var_type, IntType)
    assert isinstance(func.body[0].initializer, IntLiteral)


def test_parse_var_decl_no_init():
    """Test parsing variable declaration without initializer."""
    code = "int main() { int x; return 0; }"
    program = parse_code(code)
    func = program.functions[0]
    assert isinstance(func.body[0], VarDecl)
    assert func.body[0].initializer is None


def test_parse_assignment():
    """Test parsing assignment statement."""
    code = "int main() { int x = 0; x = 10; return x; }"
    program = parse_code(code)
    func = program.functions[0]
    assert isinstance(func.body[1], Assignment)
    assert func.body[1].name == "x"
    assert isinstance(func.body[1].value, IntLiteral)


def test_parse_if_statement():
    """Test parsing if statement."""
    code = """
    int main() {
        if (true) {
            return 1;
        }
        return 0;
    }
    """
    program = parse_code(code)
    func = program.functions[0]
    assert isinstance(func.body[0], IfStatement)
    assert isinstance(func.body[0].condition, BoolLiteral)
    assert len(func.body[0].then_branch) == 1
    assert func.body[0].else_branch is None


def test_parse_if_else_statement():
    """Test parsing if-else statement."""
    code = """
    int main() {
        if (true) {
            return 1;
        } else {
            return 0;
        }
    }
    """
    program = parse_code(code)
    func = program.functions[0]
    if_stmt = func.body[0]
    assert isinstance(if_stmt, IfStatement)
    assert len(if_stmt.then_branch) == 1
    assert len(if_stmt.else_branch) == 1


def test_parse_while_statement():
    """Test parsing while statement."""
    code = """
    int main() {
        int x = 0;
        while (x < 10) {
            x = x + 1;
        }
        return x;
    }
    """
    program = parse_code(code)
    func = program.functions[0]
    assert isinstance(func.body[1], WhileStatement)
    assert isinstance(func.body[1].condition, BinaryOp)
    assert len(func.body[1].body) == 1


def test_parse_function_with_params():
    """Test parsing function with parameters."""
    code = "int add(int a, int b) { return a + b; }"
    program = parse_code(code)
    func = program.functions[0]
    assert func.name == "add"
    assert len(func.parameters) == 2
    assert func.parameters[0][1] == "a"
    assert func.parameters[1][1] == "b"


def test_parse_multiple_functions():
    """Test parsing multiple functions."""
    code = """
    int foo() { return 1; }
    int bar() { return 2; }
    """
    program = parse_code(code)
    assert len(program.functions) == 2
    assert program.functions[0].name == "foo"
    assert program.functions[1].name == "bar"


def test_parse_comparison_operators():
    """Test parsing comparison operators."""
    code = "bool test() { return 1 < 2; }"
    program = parse_code(code)
    func = program.functions[0]
    expr = func.body[0].value
    assert isinstance(expr, BinaryOp)
    assert expr.op == '<'


def test_parse_logical_operators():
    """Test parsing logical operators."""
    code = "bool test() { return true && false; }"
    program = parse_code(code)
    func = program.functions[0]
    expr = func.body[0].value
    assert isinstance(expr, BinaryOp)
    assert expr.op == '&&'


def test_parse_missing_semicolon():
    """Test error on missing semicolon."""
    code = "int main() { return 42 }"
    with pytest.raises(ParserError) as exc:
        parse_code(code)
    assert "Expected SEMICOLON" in str(exc.value)


def test_parse_unmatched_brace():
    """Test error on unmatched brace."""
    code = "int main() { return 42;"
    with pytest.raises(ParserError) as exc:
        parse_code(code)
    assert "Expected RBRACE" in str(exc.value)
