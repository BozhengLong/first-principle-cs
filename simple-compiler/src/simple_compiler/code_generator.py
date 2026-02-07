"""Code Generator - converts IR to bytecode."""

from typing import Dict
from .ir import *
from .bytecode import Bytecode, Opcode


class CodeGenerator:
    """Generate bytecode from IR."""

    def __init__(self):
        self.bytecode = Bytecode()
        self.variables: Dict[str, int] = {}  # variable -> stack offset
        self.var_counter = 0

    def generate(self, instructions: List[IRInstruction]) -> Bytecode:
        """Generate bytecode from IR instructions."""
        for instr in instructions:
            self.generate_instruction(instr)

        # Add HALT at the end
        self.bytecode.add(Opcode.HALT)

        # Resolve label references
        self.bytecode.resolve_labels()

        return self.bytecode

    def generate_instruction(self, instr: IRInstruction) -> None:
        """Generate bytecode for a single IR instruction."""
        if isinstance(instr, LabelIR):
            self.bytecode.add_label(instr.label)

        elif isinstance(instr, BinaryIR):
            self.generate_binary(instr)

        elif isinstance(instr, UnaryIR):
            self.generate_unary(instr)

        elif isinstance(instr, CopyIR):
            self.generate_copy(instr)

        elif isinstance(instr, JumpIR):
            self.bytecode.add(Opcode.JMP, instr.label)

        elif isinstance(instr, ConditionalJumpIR):
            # Load condition
            self.load_value(instr.condition)
            # Jump if NOT zero (condition is true)
            self.bytecode.add(Opcode.JNZ, instr.label)

        elif isinstance(instr, CallIR):
            self.bytecode.add(Opcode.CALL, f"func_{instr.function}")
            if instr.dest:
                self.store_variable(instr.dest)

        elif isinstance(instr, ReturnIR):
            if instr.value:
                self.load_value(instr.value)
            self.bytecode.add(Opcode.RET)

        elif isinstance(instr, ParamIR):
            # For now, parameters are handled by CALL
            pass

    def generate_binary(self, instr: BinaryIR) -> None:
        """Generate bytecode for binary operation."""
        # Load operands
        self.load_value(instr.left)
        self.load_value(instr.right)

        # Generate operation
        op_map = {
            '+': Opcode.ADD,
            '-': Opcode.SUB,
            '*': Opcode.MUL,
            '/': Opcode.DIV,
            '%': Opcode.MOD,
            '<': Opcode.LT,
            '>': Opcode.GT,
            '<=': Opcode.LE,
            '>=': Opcode.GE,
            '==': Opcode.EQ,
            '!=': Opcode.NE,
            '&&': Opcode.AND,
            '||': Opcode.OR,
        }

        if instr.op in op_map:
            self.bytecode.add(op_map[instr.op])
        else:
            raise Exception(f"Unknown binary operator: {instr.op}")

        # Store result
        self.store_variable(instr.dest)

    def generate_unary(self, instr: UnaryIR) -> None:
        """Generate bytecode for unary operation."""
        self.load_value(instr.operand)

        if instr.op == '-':
            self.bytecode.add(Opcode.NEG)
        elif instr.op == '!':
            self.bytecode.add(Opcode.NOT)
        else:
            raise Exception(f"Unknown unary operator: {instr.op}")

        self.store_variable(instr.dest)

    def generate_copy(self, instr: CopyIR) -> None:
        """Generate bytecode for copy operation."""
        self.load_value(instr.source)
        self.store_variable(instr.dest)

    def load_value(self, value: str) -> None:
        """Load a value onto the stack."""
        # Check if it's a constant
        if value.lstrip('-').isdigit():
            self.bytecode.add(Opcode.PUSH, int(value))
        else:
            # It's a variable
            self.bytecode.add(Opcode.LOAD, value)

    def store_variable(self, var: str) -> None:
        """Store top of stack to variable."""
        self.bytecode.add(Opcode.STORE, var)
