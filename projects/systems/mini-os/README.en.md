[中文](README.md) | [English](README.en.md)

---

# mini-os

A minimal operating system kernel for learning OS fundamentals.

## Overview

mini-os is a teaching operating system that implements core OS abstractions:
- Process scheduling and management
- Virtual memory management
- System call interface
- Context switching
- Interrupt handling

This project is part of Module C (Single-Machine Systems) in the first-principles-cs learning path.

## Features

- **Process Management**: Time-slice round-robin scheduler with process creation (fork), execution (exec), and termination
- **Virtual Memory**: Two-level page tables with process isolation and memory protection
- **System Calls**: Unix-like system call interface (fork, exec, wait, exit, read, write, etc.)
- **Interrupt Handling**: Hardware interrupts (timer) and software interrupts (system calls, exceptions)
- **User Programs**: Simple shell, echo, and test programs

## Architecture

- **Target Platform**: x86 (32-bit)
- **Language**: C
- **Emulator**: QEMU
- **Bootloader**: GRUB or simple boot sector

## Building

### Prerequisites

- GCC (i386 cross-compiler)
- QEMU (qemu-system-i386)
- GDB (for debugging)
- Make

### Build Commands

```bash
# Build the kernel
make

# Run in QEMU
make qemu

# Run with GDB debugging
make qemu-gdb

# Run tests
make test

# Clean build artifacts
make clean
```

## Project Structure

```
mini-os/
├── kernel/          # Kernel code
│   ├── boot/        # Boot loader
│   ├── proc/        # Process management
│   ├── mm/          # Memory management
│   ├── trap/        # Interrupt handling
│   └── lib/         # Kernel library
├── user/            # User-space programs
│   ├── lib/         # User library
│   └── programs/    # Example programs
├── tests/           # Test suite
└── docs/            # Documentation
```

## Documentation

- [Design Document](docs/design.md) — System design and architecture
- [Architecture](docs/architecture.md) — Component architecture
- [System Call Specification](docs/syscall-spec.md) — System call interface

## Testing

mini-os includes comprehensive testing:
- **Unit Tests**: Test individual components (scheduler, memory manager, etc.)
- **Integration Tests**: End-to-end testing of system functionality
- **Fault Injection Tests**: Test error handling and recovery
- **Benchmark Tests**: Performance measurements

## Learning Resources

This project is inspired by:
- **xv6**: MIT's teaching operating system
- **MIT 6.828**: Operating System Engineering course
- **"Operating Systems: Three Easy Pieces"** by Remzi & Andrea

## License

MIT License — see LICENSE file for details

## Related Course

This project is part of the [first-principles-cs](https://github.com/first-principles-cs/guide) curriculum — Module C: Single-Machine Systems.
