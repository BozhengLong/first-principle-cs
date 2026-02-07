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

## Features

### Phase 1: Basic Structure (Completed)
- Disk I/O simulation using files as virtual disks
- Block allocator with bitmap-based free space tracking
- Superblock management
- File system creation tool (mkfs)

### Phase 2: Inode and Directory (Completed)
- Inode allocation and management with caching
- Directory operations (lookup, link, unlink)
- Path resolution (absolute paths)
- File creation and deletion

### Phase 3: Buffer Cache (Completed)
- Buffer cache with LRU eviction policy
- Reference counting to prevent eviction of in-use buffers
- Write-through policy for data integrity

### Phase 4: Journaling (Completed)
- Write-ahead logging for crash consistency
- Transaction interface (begin_op/end_op)
- Automatic recovery on mount
- Group commit optimization

### Phase 5: Interactive Shell (Completed)
- Full-featured shell with commands: ls, mkdir, touch, rm, cat, write, append, stat, pwd, cd
- Path normalization (handling . and ..)
- Working directory support

## Quick Start

### Build

```bash
make
```

### Create a File System

```bash
./mkfs fs.img
```

This creates a 4MB file system image with:
- 1000 blocks (4KB each)
- 30 log blocks
- 333 inode blocks
- 1 bitmap block
- 634 data blocks

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
- `ls [path]` - List directory contents
- `mkdir <path>` - Create directory
- `touch <path>` - Create empty file
- `rm <path>` - Delete file/directory
- `cat <path>` - Display file contents
- `write <path> <text>` - Write text to file
- `pwd` - Print working directory
- `cd <path>` - Change directory
- `stat <path>` - Show file information
- `exit` - Exit shell

## Architecture

### Disk Layout

```
| Boot | Super | Log | Inodes | Bitmap | Data blocks |
|  0   |   1   | 2-31| 32-364 |  365   |  366-999    |
```

### Core Data Structures

**Superblock** (Block 1):
- Magic number for validation
- File system size and layout information
- Pointers to log, inode, and bitmap regions

**Inode** (64 bytes):
- File type (file/directory/device)
- Size and link count
- 12 direct block pointers
- 1 indirect block pointer

**Directory Entry** (16 bytes):
- Inode number
- File name (14 characters max)

**Block Allocator**:
- Bitmap-based free space tracking
- One bit per block
- Supports allocation and deallocation

## Implementation Details

### Disk I/O Simulation

The disk is simulated using a regular file. Each block is 4KB (4096 bytes). The disk module provides:

- `disk_init()`: Create a new disk image
- `disk_open()`: Open an existing disk image
- `disk_read()`: Read a block
- `disk_write()`: Write a block
- `disk_sync()`: Flush all writes to disk
- `disk_close()`: Close the disk

### Block Allocator

The block allocator uses a bitmap to track free blocks:

- `binit()`: Initialize the allocator with superblock info
- `balloc()`: Allocate a free block (returns block number)
- `bfree()`: Free an allocated block
- `bzeroblk()`: Zero out a block

The bitmap is stored on disk and updated atomically.

## Testing

### Unit Tests

All 5 phases include comprehensive unit tests (31 total):

**Phase 1** (3 tests):
- Disk initialization and I/O
- Block allocation and deallocation
- Bitmap consistency

**Phase 2** (9 tests):
- Inode allocation and caching
- Inode I/O operations
- Block mapping (direct and indirect)
- Directory operations
- Path resolution
- File creation and deletion
- Integration tests

**Phase 3** (6 tests):
- Buffer read/release (bread/brelse)
- Cache hit verification
- Write-through policy
- LRU eviction
- Reference counting prevents eviction
- Phase 2 regression

**Phase 4** (5 tests):
- Basic transaction
- Multi-operation transaction
- Log header cleanup
- Unlink with transaction
- Phase 3 regression

**Phase 5** (8 tests):
- Path normalization
- mkdir/touch basic operations
- Write and cat
- ls directory listing
- rm file deletion
- Type string conversion
- Phase 4 regression

Run all tests with:
```bash
make test
```

### Test Results

```
=== All Phases ===
Phase 1: 3 passed
Phase 2: 9 passed
Phase 3: 6 passed
Phase 4: 5 passed
Phase 5: 8 passed

Total: 31 tests passed
```

## Project Structure

```
simple-fs/
├── README.md                 # This file
├── Makefile                  # Build system
├── src/
│   ├── types.h              # Type definitions
│   ├── param.h              # Parameters and constants
│   ├── fs.h                 # File system structures
│   ├── disk.c/h             # Disk I/O simulation (Phase 1)
│   ├── block.c/h            # Block allocator (Phase 1)
│   ├── mkfs.c               # File system creation tool (Phase 1)
│   ├── lock.c/h             # Lock infrastructure (Phase 2)
│   ├── inode.c/h            # Inode management (Phase 2)
│   ├── dir.c/h              # Directory operations (Phase 2)
│   ├── namei.c/h            # Path resolution (Phase 2)
│   ├── file.c/h             # File operations (Phase 2)
│   ├── buf.c/h              # Buffer cache (Phase 3)
│   ├── log.c/h              # Journaling (Phase 4)
│   └── shell.c/h            # Interactive shell (Phase 5)
├── tests/
│   └── unit/
│       ├── test_phase1.c    # Phase 1 unit tests
│       ├── test_phase2.c    # Phase 2 unit tests
│       ├── test_phase3.c    # Phase 3 unit tests
│       ├── test_phase4.c    # Phase 4 unit tests
│       └── test_phase5.c    # Phase 5 unit tests
└── docs/                     # Documentation
```

## Development Roadmap

### Phase 1: Basic Structure ✅ (Completed)
- [x] Project structure and build system
- [x] Core data structures
- [x] Disk I/O simulation
- [x] Block allocator
- [x] mkfs tool
- [x] Unit tests (3 tests)

### Phase 2: Inode and Directory ✅ (Completed)
- [x] Inode allocation/deallocation
- [x] Inode read/write operations
- [x] Directory operations (lookup, link, unlink)
- [x] Path resolution
- [x] File creation/deletion
- [x] Unit tests (9 tests)

### Phase 3: Buffer Cache ✅ (Completed)
- [x] Buffer cache with LRU eviction
- [x] Reference counting
- [x] Write-through policy
- [x] Unit tests (6 tests)

### Phase 4: Journaling ✅ (Completed)
- [x] Log structure
- [x] Transaction interface (begin_op/end_op)
- [x] Commit and recovery
- [x] Integration with all write operations
- [x] Unit tests (5 tests)

### Phase 5: Interactive Shell ✅ (Completed)
- [x] Shell command parser
- [x] File system commands (ls, mkdir, touch, rm, cat, write, append, stat)
- [x] Navigation commands (pwd, cd)
- [x] Path normalization
- [x] Unit tests (8 tests)

### Phase 6: Documentation (Optional)
- [ ] Design documentation
- [ ] API specification
- [ ] Implementation guide

## Design Principles

1. **Simplicity**: Keep the implementation minimal and understandable
2. **Correctness**: Ensure crash consistency through journaling
3. **Verification**: Test thoroughly with multiple strategies
4. **Education**: Code should teach file system concepts

## References

- **xv6**: MIT's teaching operating system
  - https://github.com/mit-pdos/xv6-public
  - https://pdos.csail.mit.edu/6.828/2018/xv6/book-rev11.pdf

- **Journaling**: "Journaling the Linux ext2fs Filesystem" by Stephen Tweedie (1998)

- **OSTEP**: "Operating Systems: Three Easy Pieces" - File System chapters
  - https://pages.cs.wisc.edu/~remzi/OSTEP/

## License

MIT License - See LICENSE file for details

## Author

Part of the First Principles CS project - Q2 Project 2
