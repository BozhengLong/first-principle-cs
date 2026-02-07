"""Virtual Machine - executes bytecode."""

from typing import Dict, List, Any
from .bytecode import Bytecode, Opcode, BytecodeInstruction
from .errors import RuntimeError as VMRuntimeError


class VM:
    """Stack-based virtual machine."""

    def __init__(self, bytecode: Bytecode):
        self.bytecode = bytecode
        self.stack: List[int] = []
        self.variables: Dict[str, int] = {}
        self.pc = 0  # Program counter
        self.call_stack: List[int] = []  # Return addresses

    def execute(self) -> int:
        """Execute the bytecode program."""
        # Find main function
        if "func_main" not in self.bytecode.labels:
            raise VMRuntimeError("No main function found", 0, 0)

        # Jump to main
        self.pc = self.bytecode.labels["func_main"]

        # Execute until HALT or RET from main
        while self.pc < len(self.bytecode.instructions):
            instr = self.bytecode.instructions[self.pc]
            self.execute_instruction(instr)

            # Check if we returned from main
            if instr.opcode == Opcode.RET and len(self.call_stack) == 0:
                break

        # Return value from stack (or 0 if empty)
        if self.stack:
            return self.stack[-1]
        return 0

    def execute_instruction(self, instr: BytecodeInstruction) -> None:
        """Execute a single instruction."""
        opcode = instr.opcode
        operand = instr.operand

        if opcode == Opcode.PUSH:
            self.stack.append(operand)
            self.pc += 1

        elif opcode == Opcode.LOAD:
            if operand not in self.variables:
                raise VMRuntimeError(f"Undefined variable: {operand}", 0, 0)
            self.stack.append(self.variables[operand])
            self.pc += 1

        elif opcode == Opcode.STORE:
            if not self.stack:
                raise VMRuntimeError("Stack underflow", 0, 0)
            self.variables[operand] = self.stack.pop()
            self.pc += 1

        elif opcode == Opcode.ADD:
            self.binary_op(lambda a, b: a + b)

        elif opcode == Opcode.SUB:
            self.binary_op(lambda a, b: a - b)

        elif opcode == Opcode.MUL:
            self.binary_op(lambda a, b: a * b)

        elif opcode == Opcode.DIV:
            if self.stack[-1] == 0:
                raise VMRuntimeError("Division by zero", 0, 0)
            self.binary_op(lambda a, b: a // b)

        elif opcode == Opcode.MOD:
            if self.stack[-1] == 0:
                raise VMRuntimeError("Modulo by zero", 0, 0)
            self.binary_op(lambda a, b: a % b)

        elif opcode == Opcode.NEG:
            if not self.stack:
                raise VMRuntimeError("Stack underflow", 0, 0)
            self.stack[-1] = -self.stack[-1]
            self.pc += 1

        elif opcode == Opcode.LT:
            self.binary_op(lambda a, b: 1 if a < b else 0)

        elif opcode == Opcode.GT:
            self.binary_op(lambda a, b: 1 if a > b else 0)

        elif opcode == Opcode.LE:
            self.binary_op(lambda a, b: 1 if a <= b else 0)

        elif opcode == Opcode.GE:
            self.binary_op(lambda a, b: 1 if a >= b else 0)

        elif opcode == Opcode.EQ:
            self.binary_op(lambda a, b: 1 if a == b else 0)

        elif opcode == Opcode.NE:
            self.binary_op(lambda a, b: 1 if a != b else 0)

        elif opcode == Opcode.AND:
            self.binary_op(lambda a, b: 1 if (a != 0 and b != 0) else 0)

        elif opcode == Opcode.OR:
            self.binary_op(lambda a, b: 1 if (a != 0 or b != 0) else 0)

        elif opcode == Opcode.NOT:
            if not self.stack:
                raise VMRuntimeError("Stack underflow", 0, 0)
            self.stack[-1] = 1 if self.stack[-1] == 0 else 0
            self.pc += 1

        elif opcode == Opcode.JMP:
            self.pc = operand

        elif opcode == Opcode.JZ:
            if not self.stack:
                raise VMRuntimeError("Stack underflow", 0, 0)
            value = self.stack.pop()
            if value == 0:
                self.pc = operand
            else:
                self.pc += 1

        elif opcode == Opcode.JNZ:
            if not self.stack:
                raise VMRuntimeError("Stack underflow", 0, 0)
            value = self.stack.pop()
            if value != 0:
                self.pc = operand
            else:
                self.pc += 1

        elif opcode == Opcode.CALL:
            self.call_stack.append(self.pc + 1)
            self.pc = operand

        elif opcode == Opcode.RET:
            if self.call_stack:
                self.pc = self.call_stack.pop()
            else:
                # Return from main
                self.pc = len(self.bytecode.instructions)

        elif opcode == Opcode.HALT:
            self.pc = len(self.bytecode.instructions)

        elif opcode == Opcode.POP:
            if self.stack:
                self.stack.pop()
            self.pc += 1

        else:
            raise VMRuntimeError(f"Unknown opcode: {opcode}", 0, 0)

    def binary_op(self, op) -> None:
        """Execute a binary operation."""
        if len(self.stack) < 2:
            raise VMRuntimeError("Stack underflow", 0, 0)
        b = self.stack.pop()
        a = self.stack.pop()
        self.stack.append(op(a, b))
        self.pc += 1
