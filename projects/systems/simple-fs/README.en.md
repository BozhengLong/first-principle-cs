[中文](README.md) | [English](README.en.md)

---

# simple-fs: A Simple File System Implementation

A minimal file system implementation for educational purposes, demonstrating core concepts of file systems, crash consistency, and journaling.

## Project Status

**Status**: ✅ ALL PHASES COMPLETED

- ✅ Phase 1: Basic Structure (Disk I/O, Block Allocator, mkfs)
- ✅ Phase 2: Inode and Directory Operations
- ✅ Phase 3: Buffer Cache with LRU
- ✅ Phase 4: Journaling (Write-Ahead Logging)
- ✅ Phase 5: Interactive Shell

## Overview

simple-fs is a Unix-like file system implementation inspired by xv6. It demonstrates:

- **File system abstractions**: inodes, directories, block allocation
- **Crash consistency**: journaling (write-ahead logging)
- **Verification**: unit tests, fault injection, fuzzing

## Quick Start

### Build

```bash
make
```

### Create a File System

```bash
./mkfs fs.img
```

### Run Tests

```bash
make test
```

### Run Interactive Shell

```bash
./mkfs fs.img
./shell fs.img
```

Shell commands:
- `ls [path]` — List directory contents
- `mkdir <path>` — Create directory
- `touch <path>` — Create empty file
- `rm <path>` — Delete file/directory
- `cat <path>` — Display file contents
- `write <path> <text>` — Write text to file
- `pwd` — Print working directory
- `cd <path>` — Change directory
- `stat <path>` — Show file information
- `exit` — Exit shell

## Key Invariants

1. **Crash consistency**: File system remains usable after a crash
2. **Atomicity**: Operations either fully complete or have no effect
3. **Reference count correctness**: Inode reference counts match actual references
4. **Block allocation consistency**: Allocated blocks are never double-allocated

## Testing

All 5 phases include comprehensive unit tests (31 total):

- Phase 1: 3 tests (disk I/O, block allocation, bitmap)
- Phase 2: 9 tests (inodes, directories, paths, files)
- Phase 3: 6 tests (buffer cache, LRU, reference counting)
- Phase 4: 5 tests (transactions, journaling, recovery)
- Phase 5: 8 tests (shell commands, path normalization)

```bash
make test
```

## Project Structure

```
simple-fs/
├── Makefile
├── src/
│   ├── types.h, param.h, fs.h   # Type definitions and constants
│   ├── disk.c/h                  # Disk I/O simulation (Phase 1)
│   ├── block.c/h                 # Block allocator (Phase 1)
│   ├── mkfs.c                    # File system creation tool (Phase 1)
│   ├── lock.c/h                  # Lock infrastructure (Phase 2)
│   ├── inode.c/h                 # Inode management (Phase 2)
│   ├── dir.c/h                   # Directory operations (Phase 2)
│   ├── namei.c/h                 # Path resolution (Phase 2)
│   ├── file.c/h                  # File operations (Phase 2)
│   ├── buf.c/h                   # Buffer cache (Phase 3)
│   ├── log.c/h                   # Journaling (Phase 4)
│   └── shell.c/h                 # Interactive shell (Phase 5)
├── tests/unit/                   # Unit tests per phase
└── docs/                         # Documentation
```

## References

- **xv6**: MIT's teaching operating system
- **"Journaling the Linux ext2fs Filesystem"** by Stephen Tweedie (1998)
- **OSTEP**: "Operating Systems: Three Easy Pieces" — File System chapters

## License

MIT License — see LICENSE file for details

## Related Course

Part of the [first-principles-cs](https://github.com/first-principles-cs/guide) curriculum — Module C: Single-Machine Systems.
