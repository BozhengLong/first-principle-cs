"""Type checker for SimpleLang."""

from typing import Optional
from .ast_nodes import *
from .symbol_table import SymbolTable
from .types import FunctionSignature, types_equal, type_to_string
from .errors import TypeError as TypeCheckError


class TypeChecker:
    """Type checker for SimpleLang programs."""

    def __init__(self):
        self.symbol_table = SymbolTable()
        self.current_function: Optional[FunctionSignature] = None

    def check_program(self, program: Program) -> None:
        """Type check the entire program."""
        # First pass: collect all function signatures
        for func in program.functions:
            signature = FunctionSignature(
                name=func.name,
                return_type=func.return_type,
                parameter_types=[param[0] for param in func.parameters],
                parameter_names=[param[1] for param in func.parameters]
            )
            try:
                self.symbol_table.define_function(signature)
            except Exception as e:
                raise TypeCheckError(str(e), func.line, func.column)

        # Second pass: type check function bodies
        for func in program.functions:
            self.check_function(func)

    def check_function(self, func: FunctionDecl) -> None:
        """Type check a function declaration."""
        # Set current function for return type checking
        signature = self.symbol_table.lookup_function(func.name)
        self.current_function = signature

        # Enter function scope
        self.symbol_table = self.symbol_table.enter_scope()

        # Add parameters to scope
        for param_type, param_name in func.parameters:
            try:
                self.symbol_table.define_variable(param_name, param_type)
            except Exception as e:
                raise TypeCheckError(str(e), func.line, func.column)

        # Check function body
        for stmt in func.body:
            self.check_statement(stmt)

        # Exit function scope
        self.symbol_table = self.symbol_table.exit_scope()
        self.current_function = None

    def check_statement(self, stmt: Statement) -> None:
        """Type check a statement."""
        if isinstance(stmt, VarDecl):
            self.check_var_decl(stmt)
        elif isinstance(stmt, Assignment):
            self.check_assignment(stmt)
        elif isinstance(stmt, IfStatement):
            self.check_if_statement(stmt)
        elif isinstance(stmt, WhileStatement):
            self.check_while_statement(stmt)
        elif isinstance(stmt, ReturnStatement):
            self.check_return_statement(stmt)
        elif isinstance(stmt, ExpressionStatement):
            self.check_expression(stmt.expression)
        else:
            raise TypeCheckError(
                f"Unknown statement type: {type(stmt).__name__}",
                stmt.line,
                stmt.column
            )

    def check_var_decl(self, stmt: VarDecl) -> None:
        """Type check variable declaration."""
        # Check initializer if present
        if stmt.initializer:
            init_type = self.check_expression(stmt.initializer)
            if not types_equal(stmt.var_type, init_type):
                raise TypeCheckError(
                    f"Type mismatch in variable declaration: expected {type_to_string(stmt.var_type)}, "
                    f"got {type_to_string(init_type)}",
                    stmt.line,
                    stmt.column
                )

        # Add variable to symbol table
        try:
            self.symbol_table.define_variable(stmt.name, stmt.var_type)
        except Exception as e:
            raise TypeCheckError(str(e), stmt.line, stmt.column)

    def check_assignment(self, stmt: Assignment) -> None:
        """Type check assignment statement."""
        # Look up variable
        var_type = self.symbol_table.lookup_variable(stmt.name)
        if var_type is None:
            raise TypeCheckError(
                f"Undefined variable: '{stmt.name}'",
                stmt.line,
                stmt.column
            )

        # Check value type
        value_type = self.check_expression(stmt.value)
        if not types_equal(var_type, value_type):
            raise TypeCheckError(
                f"Type mismatch in assignment: expected {type_to_string(var_type)}, "
                f"got {type_to_string(value_type)}",
                stmt.line,
                stmt.column
            )

    def check_if_statement(self, stmt: IfStatement) -> None:
        """Type check if statement."""
        # Check condition is boolean
        cond_type = self.check_expression(stmt.condition)
        if not isinstance(cond_type, BoolType):
            raise TypeCheckError(
                f"If condition must be boolean, got {type_to_string(cond_type)}",
                stmt.line,
                stmt.column
            )

        # Check then branch
        self.symbol_table = self.symbol_table.enter_scope()
        for s in stmt.then_branch:
            self.check_statement(s)
        self.symbol_table = self.symbol_table.exit_scope()

        # Check else branch if present
        if stmt.else_branch:
            self.symbol_table = self.symbol_table.enter_scope()
            for s in stmt.else_branch:
                self.check_statement(s)
            self.symbol_table = self.symbol_table.exit_scope()

    def check_while_statement(self, stmt: WhileStatement) -> None:
        """Type check while statement."""
        # Check condition is boolean
        cond_type = self.check_expression(stmt.condition)
        if not isinstance(cond_type, BoolType):
            raise TypeCheckError(
                f"While condition must be boolean, got {type_to_string(cond_type)}",
                stmt.line,
                stmt.column
            )

        # Check body
        self.symbol_table = self.symbol_table.enter_scope()
        for s in stmt.body:
            self.check_statement(s)
        self.symbol_table = self.symbol_table.exit_scope()

    def check_return_statement(self, stmt: ReturnStatement) -> None:
        """Type check return statement."""
        if self.current_function is None:
            raise TypeCheckError(
                "Return statement outside of function",
                stmt.line,
                stmt.column
            )

        expected_type = self.current_function.return_type

        if stmt.value is None:
            # Return with no value
            if not isinstance(expected_type, VoidType):
                raise TypeCheckError(
                    f"Function must return {type_to_string(expected_type)}",
                    stmt.line,
                    stmt.column
                )
        else:
            # Return with value
            if isinstance(expected_type, VoidType):
                raise TypeCheckError(
                    "Void function cannot return a value",
                    stmt.line,
                    stmt.column
                )

            return_type = self.check_expression(stmt.value)
            if not types_equal(expected_type, return_type):
                raise TypeCheckError(
                    f"Return type mismatch: expected {type_to_string(expected_type)}, "
                    f"got {type_to_string(return_type)}",
                    stmt.line,
                    stmt.column
                )

    def check_expression(self, expr: Expression) -> Type:
        """Type check an expression and return its type."""
        if isinstance(expr, IntLiteral):
            return IntType(expr.line, expr.column)

        elif isinstance(expr, BoolLiteral):
            return BoolType(expr.line, expr.column)

        elif isinstance(expr, Variable):
            var_type = self.symbol_table.lookup_variable(expr.name)
            if var_type is None:
                raise TypeCheckError(
                    f"Undefined variable: '{expr.name}'",
                    expr.line,
                    expr.column
                )
            return var_type

        elif isinstance(expr, BinaryOp):
            return self.check_binary_op(expr)

        elif isinstance(expr, UnaryOp):
            return self.check_unary_op(expr)

        elif isinstance(expr, FunctionCall):
            return self.check_function_call(expr)

        else:
            raise TypeCheckError(
                f"Unknown expression type: {type(expr).__name__}",
                expr.line,
                expr.column
            )

    def check_binary_op(self, expr: BinaryOp) -> Type:
        """Type check binary operation."""
        left_type = self.check_expression(expr.left)
        right_type = self.check_expression(expr.right)

        # Arithmetic operators: int × int → int
        if expr.op in ['+', '-', '*', '/', '%']:
            if not isinstance(left_type, IntType):
                raise TypeCheckError(
                    f"Left operand of '{expr.op}' must be int, got {type_to_string(left_type)}",
                    expr.line,
                    expr.column
                )
            if not isinstance(right_type, IntType):
                raise TypeCheckError(
                    f"Right operand of '{expr.op}' must be int, got {type_to_string(right_type)}",
                    expr.line,
                    expr.column
                )
            return IntType(expr.line, expr.column)

        # Comparison operators: int × int → bool
        elif expr.op in ['<', '>', '<=', '>=']:
            if not isinstance(left_type, IntType):
                raise TypeCheckError(
                    f"Left operand of '{expr.op}' must be int, got {type_to_string(left_type)}",
                    expr.line,
                    expr.column
                )
            if not isinstance(right_type, IntType):
                raise TypeCheckError(
                    f"Right operand of '{expr.op}' must be int, got {type_to_string(right_type)}",
                    expr.line,
                    expr.column
                )
            return BoolType(expr.line, expr.column)

        # Equality operators: T × T → bool (same types)
        elif expr.op in ['==', '!=']:
            if not types_equal(left_type, right_type):
                raise TypeCheckError(
                    f"Operands of '{expr.op}' must have same type, "
                    f"got {type_to_string(left_type)} and {type_to_string(right_type)}",
                    expr.line,
                    expr.column
                )
            return BoolType(expr.line, expr.column)

        # Logical operators: bool × bool → bool
        elif expr.op in ['&&', '||']:
            if not isinstance(left_type, BoolType):
                raise TypeCheckError(
                    f"Left operand of '{expr.op}' must be bool, got {type_to_string(left_type)}",
                    expr.line,
                    expr.column
                )
            if not isinstance(right_type, BoolType):
                raise TypeCheckError(
                    f"Right operand of '{expr.op}' must be bool, got {type_to_string(right_type)}",
                    expr.line,
                    expr.column
                )
            return BoolType(expr.line, expr.column)

        else:
            raise TypeCheckError(
                f"Unknown binary operator: '{expr.op}'",
                expr.line,
                expr.column
            )

    def check_unary_op(self, expr: UnaryOp) -> Type:
        """Type check unary operation."""
        operand_type = self.check_expression(expr.operand)

        # Negation: -int → int
        if expr.op == '-':
            if not isinstance(operand_type, IntType):
                raise TypeCheckError(
                    f"Operand of '-' must be int, got {type_to_string(operand_type)}",
                    expr.line,
                    expr.column
                )
            return IntType(expr.line, expr.column)

        # Logical NOT: !bool → bool
        elif expr.op == '!':
            if not isinstance(operand_type, BoolType):
                raise TypeCheckError(
                    f"Operand of '!' must be bool, got {type_to_string(operand_type)}",
                    expr.line,
                    expr.column
                )
            return BoolType(expr.line, expr.column)

        else:
            raise TypeCheckError(
                f"Unknown unary operator: '{expr.op}'",
                expr.line,
                expr.column
            )

    def check_function_call(self, expr: FunctionCall) -> Type:
        """Type check function call."""
        # Look up function
        func_sig = self.symbol_table.lookup_function(expr.name)
        if func_sig is None:
            raise TypeCheckError(
                f"Undefined function: '{expr.name}'",
                expr.line,
                expr.column
            )

        # Check argument count
        if len(expr.arguments) != len(func_sig.parameter_types):
            raise TypeCheckError(
                f"Function '{expr.name}' expects {len(func_sig.parameter_types)} arguments, "
                f"got {len(expr.arguments)}",
                expr.line,
                expr.column
            )

        # Check argument types
        for i, (arg, expected_type) in enumerate(zip(expr.arguments, func_sig.parameter_types)):
            arg_type = self.check_expression(arg)
            if not types_equal(arg_type, expected_type):
                raise TypeCheckError(
                    f"Argument {i+1} of function '{expr.name}': expected {type_to_string(expected_type)}, "
                    f"got {type_to_string(arg_type)}",
                    expr.line,
                    expr.column
                )

        return func_sig.return_type
