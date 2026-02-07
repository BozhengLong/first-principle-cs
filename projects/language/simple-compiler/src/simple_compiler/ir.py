"""Intermediate Representation (IR) definitions."""

from dataclasses import dataclass
from typing import Optional, List


@dataclass
class IRInstruction:
    """Base class for IR instructions."""
    pass


@dataclass
class BinaryIR(IRInstruction):
    """Binary operation: dest = left op right"""
    dest: str
    op: str
    left: str
    right: str

    def __repr__(self) -> str:
        return f"{self.dest} = {self.left} {self.op} {self.right}"


@dataclass
class UnaryIR(IRInstruction):
    """Unary operation: dest = op operand"""
    dest: str
    op: str
    operand: str

    def __repr__(self) -> str:
        return f"{self.dest} = {self.op}{self.operand}"


@dataclass
class CopyIR(IRInstruction):
    """Copy: dest = source"""
    dest: str
    source: str

    def __repr__(self) -> str:
        return f"{self.dest} = {self.source}"


@dataclass
class LabelIR(IRInstruction):
    """Label: L1:"""
    label: str

    def __repr__(self) -> str:
        return f"{self.label}:"


@dataclass
class JumpIR(IRInstruction):
    """Unconditional jump: goto label"""
    label: str

    def __repr__(self) -> str:
        return f"goto {self.label}"


@dataclass
class ConditionalJumpIR(IRInstruction):
    """Conditional jump: if condition goto label"""
    condition: str
    label: str

    def __repr__(self) -> str:
        return f"if {self.condition} goto {self.label}"


@dataclass
class CallIR(IRInstruction):
    """Function call: dest = call func(args)"""
    dest: Optional[str]
    function: str
    arguments: List[str]

    def __repr__(self) -> str:
        args = ', '.join(self.arguments)
        if self.dest:
            return f"{self.dest} = call {self.function}({args})"
        return f"call {self.function}({args})"


@dataclass
class ReturnIR(IRInstruction):
    """Return: return value"""
    value: Optional[str]

    def __repr__(self) -> str:
        if self.value:
            return f"return {self.value}"
        return "return"


@dataclass
class ParamIR(IRInstruction):
    """Parameter: param value"""
    value: str

    def __repr__(self) -> str:
        return f"param {self.value}"
