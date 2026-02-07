"""IR Generator - converts AST to Intermediate Representation."""

from typing import List
from .ast_nodes import *
from .ir import *


class IRGenerator:
    """Generate IR from AST."""

    def __init__(self):
        self.instructions: List[IRInstruction] = []
        self.temp_counter = 0
        self.label_counter = 0

    def new_temp(self) -> str:
        """Generate a new temporary variable."""
        temp = f"t{self.temp_counter}"
        self.temp_counter += 1
        return temp

    def new_label(self) -> str:
        """Generate a new label."""
        label = f"L{self.label_counter}"
        self.label_counter += 1
        return label

    def generate(self, program: Program) -> List[IRInstruction]:
        """Generate IR for the entire program."""
        for func in program.functions:
            self.generate_function(func)
        return self.instructions

    def generate_function(self, func: FunctionDecl) -> None:
        """Generate IR for a function."""
        # Function label
        self.instructions.append(LabelIR(f"func_{func.name}"))

        # Generate body
        for stmt in func.body:
            self.generate_statement(stmt)

    def generate_statement(self, stmt: Statement) -> None:
        """Generate IR for a statement."""
        if isinstance(stmt, VarDecl):
            if stmt.initializer:
                value = self.generate_expression(stmt.initializer)
                self.instructions.append(CopyIR(stmt.name, value))

        elif isinstance(stmt, Assignment):
            value = self.generate_expression(stmt.value)
            self.instructions.append(CopyIR(stmt.name, value))

        elif isinstance(stmt, IfStatement):
            self.generate_if_statement(stmt)

        elif isinstance(stmt, WhileStatement):
            self.generate_while_statement(stmt)

        elif isinstance(stmt, ReturnStatement):
            if stmt.value:
                value = self.generate_expression(stmt.value)
                self.instructions.append(ReturnIR(value))
            else:
                self.instructions.append(ReturnIR(None))

        elif isinstance(stmt, ExpressionStatement):
            self.generate_expression(stmt.expression)

    def generate_if_statement(self, stmt: IfStatement) -> None:
        """Generate IR for if statement."""
        cond = self.generate_expression(stmt.condition)
        then_label = self.new_label()
        else_label = self.new_label()
        end_label = self.new_label()

        # If condition is true, jump to then
        self.instructions.append(ConditionalJumpIR(cond, then_label))

        # Else branch (fall through if condition is false)
        if stmt.else_branch:
            for s in stmt.else_branch:
                self.generate_statement(s)
        self.instructions.append(JumpIR(end_label))

        # Then branch
        self.instructions.append(LabelIR(then_label))
        for s in stmt.then_branch:
            self.generate_statement(s)

        self.instructions.append(LabelIR(end_label))

    def generate_while_statement(self, stmt: WhileStatement) -> None:
        """Generate IR for while statement."""
        start_label = self.new_label()
        body_label = self.new_label()
        end_label = self.new_label()

        self.instructions.append(LabelIR(start_label))
        cond = self.generate_expression(stmt.condition)
        # If condition is true, jump to body
        self.instructions.append(ConditionalJumpIR(cond, body_label))
        # Otherwise jump to end
        self.instructions.append(JumpIR(end_label))

        # Body
        self.instructions.append(LabelIR(body_label))
        for s in stmt.body:
            self.generate_statement(s)
        self.instructions.append(JumpIR(start_label))

        self.instructions.append(LabelIR(end_label))

    def generate_expression(self, expr: Expression) -> str:
        """Generate IR for an expression and return the result variable."""
        if isinstance(expr, IntLiteral):
            return str(expr.value)

        elif isinstance(expr, BoolLiteral):
            return "1" if expr.value else "0"

        elif isinstance(expr, Variable):
            return expr.name

        elif isinstance(expr, BinaryOp):
            left = self.generate_expression(expr.left)
            right = self.generate_expression(expr.right)
            result = self.new_temp()
            self.instructions.append(BinaryIR(result, expr.op, left, right))
            return result

        elif isinstance(expr, UnaryOp):
            operand = self.generate_expression(expr.operand)
            result = self.new_temp()
            self.instructions.append(UnaryIR(result, expr.op, operand))
            return result

        elif isinstance(expr, FunctionCall):
            # Generate parameters
            for arg in expr.arguments:
                arg_val = self.generate_expression(arg)
                self.instructions.append(ParamIR(arg_val))

            result = self.new_temp()
            self.instructions.append(CallIR(result, expr.name, []))
            return result

        return "0"
