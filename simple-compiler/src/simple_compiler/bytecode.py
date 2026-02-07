"""Bytecode definitions for the VM."""

from enum import Enum, auto
from dataclasses import dataclass
from typing import Any, List


class Opcode(Enum):
    """Bytecode opcodes."""
    # Stack operations
    PUSH = auto()       # Push constant onto stack
    LOAD = auto()       # Load variable onto stack
    STORE = auto()      # Store top of stack to variable

    # Arithmetic
    ADD = auto()
    SUB = auto()
    MUL = auto()
    DIV = auto()
    MOD = auto()
    NEG = auto()        # Unary negation

    # Comparison
    LT = auto()
    GT = auto()
    LE = auto()
    GE = auto()
    EQ = auto()
    NE = auto()

    # Logical
    AND = auto()
    OR = auto()
    NOT = auto()

    # Control flow
    JMP = auto()        # Unconditional jump
    JZ = auto()         # Jump if zero (false)
    JNZ = auto()        # Jump if not zero (true)

    # Functions
    CALL = auto()       # Call function
    RET = auto()        # Return from function

    # Other
    HALT = auto()       # Stop execution
    POP = auto()        # Pop and discard top of stack


@dataclass
class BytecodeInstruction:
    """A bytecode instruction."""
    opcode: Opcode
    operand: Any = None

    def __repr__(self) -> str:
        if self.operand is not None:
            return f"{self.opcode.name} {self.operand}"
        return self.opcode.name


class Bytecode:
    """Container for bytecode program."""

    def __init__(self):
        self.instructions: List[BytecodeInstruction] = []
        self.labels: dict[str, int] = {}  # label -> instruction index

    def add(self, opcode: Opcode, operand: Any = None) -> None:
        """Add an instruction."""
        self.instructions.append(BytecodeInstruction(opcode, operand))

    def add_label(self, label: str) -> None:
        """Add a label at current position."""
        self.labels[label] = len(self.instructions)

    def resolve_labels(self) -> None:
        """Resolve label references to instruction indices."""
        for i, instr in enumerate(self.instructions):
            if instr.opcode in [Opcode.JMP, Opcode.JZ, Opcode.JNZ, Opcode.CALL]:
                if isinstance(instr.operand, str) and instr.operand in self.labels:
                    instr.operand = self.labels[instr.operand]

    def __repr__(self) -> str:
        lines = []
        for i, instr in enumerate(self.instructions):
            # Find labels pointing to this instruction
            labels = [name for name, idx in self.labels.items() if idx == i]
            if labels:
                lines.append(f"{labels[0]}:")
            lines.append(f"  {i:4d}: {instr}")
        return '\n'.join(lines)
