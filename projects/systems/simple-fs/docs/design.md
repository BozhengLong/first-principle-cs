# simple-fs Design Document

## Overview

simple-fs is a minimal Unix-like file system implementation for educational purposes. It demonstrates core file system concepts including inodes, directories, block allocation, and crash consistency through journaling.

## Design Goals

1. **Educational**: Code should be clear and well-documented
2. **Minimal**: Implement only essential features
3. **Correct**: Ensure crash consistency
4. **Testable**: Comprehensive testing at all levels

## Architecture

### Disk Layout

The file system is divided into several regions:

```
Block 0:      Boot block (reserved)
Block 1:      Superblock
Blocks 2-31:  Log (30 blocks)
Blocks 32-364: Inodes (333 blocks, ~21,000 inodes)
Block 365:    Bitmap (tracks free blocks)
Blocks 366+:  Data blocks
```

### Core Components

#### 1. Disk I/O Layer (`disk.c/h`)

Simulates a disk using a file. Provides block-level read/write operations.

**Interface**:
- `disk_init(path, nblocks)`: Create new disk image
- `disk_open(path)`: Open existing disk
- `disk_read(blockno, buf)`: Read block
- `disk_write(blockno, buf)`: Write block
- `disk_sync()`: Flush to disk
- `disk_close()`: Close disk

**Implementation**: Uses standard file I/O (open, read, write, lseek, fsync).

#### 2. Block Allocator (`block.c/h`)

Manages free space using a bitmap.

**Interface**:
- `binit(sb)`: Initialize with superblock
- `balloc()`: Allocate a free block
- `bfree(blockno)`: Free a block
- `bzeroblk(blockno)`: Zero a block

**Implementation**:
- Bitmap stored on disk
- One bit per block (0 = free, 1 = allocated)
- Linear search for free blocks
- Atomic updates to bitmap

#### 3. Superblock

Contains file system metadata.

**Structure**:
```c
struct superblock {
    uint32 magic;        // FSMAGIC (0x53465346)
    uint32 size;         // Total blocks
    uint32 nblocks;      // Data blocks
    uint32 ninodes;      // Number of inodes
    uint32 nlog;         // Log blocks
    uint32 logstart;     // First log block
    uint32 inodestart;   // First inode block
    uint32 bmapstart;    // Bitmap block
};
```

#### 4. Inode (Planned - Phase 2)

Represents a file or directory.

**Structure**:
```c
struct dinode {
    uint16 type;         // T_FILE, T_DIR, T_DEV
    uint16 major;        // Device major number
    uint16 minor;        // Device minor number
    uint16 nlink;        // Link count
    uint32 size;         // File size in bytes
    uint32 addrs[NDIRECT+1];  // Block addresses
};
```

**Block Addressing**:
- 12 direct blocks (0-11): Direct pointers to data blocks
- 1 indirect block (12): Points to block of pointers

**Maximum File Size**:
- Direct: 12 × 4KB = 48KB
- Indirect: 1024 × 4KB = 4MB
- Total: ~4MB

#### 5. Directory (Planned - Phase 2)

A directory is a file containing directory entries.

**Structure**:
```c
struct dirent {
    uint16 inum;         // Inode number (0 = empty)
    char name[DIRSIZ];   // File name (14 chars)
};
```

**Operations**:
- `dirlookup(dp, name)`: Find entry by name
- `dirlink(dp, name, inum)`: Add entry
- `dirunlink(dp, name)`: Remove entry

#### 6. Buffer Cache (Planned - Phase 3)

Caches disk blocks in memory.

**Features**:
- LRU eviction policy
- Write-back or write-through
- Synchronization for concurrent access

#### 7. Logging (Planned - Phase 4)

Ensures crash consistency through write-ahead logging.

**Log Structure**:
```c
struct logheader {
    int n;               // Number of logged blocks
    int block[LOGSIZE];  // Block numbers
};
```

**Transaction Protocol**:
1. `begin_op()`: Start transaction
2. `log_write(buf)`: Log modifications
3. `end_op()`: Commit transaction

**Commit Process**:
1. Write modified blocks to log
2. Write commit record
3. Install blocks to final locations
4. Clear log

**Recovery**:
1. Read log header
2. If committed, replay log
3. Clear log

## File System Operations

### Phase 1 (Completed)

- ✅ Create file system (`mkfs`)
- ✅ Allocate/free blocks
- ✅ Read/write blocks

### Phase 2 (Planned)

- Allocate/free inodes
- Create/delete files
- Create/delete directories
- Lookup files by path

### Phase 3 (Planned)

- Read file data
- Write file data
- Truncate files
- Cache management

### Phase 4 (Planned)

- Atomic operations
- Crash recovery
- Consistency guarantees

## Invariants

The file system maintains these invariants:

1. **Superblock validity**: Magic number matches
2. **Block allocation**: Each block allocated to at most one file
3. **Inode references**: Link count matches actual references
4. **Directory consistency**: All entries point to valid inodes
5. **Bitmap consistency**: Bitmap accurately reflects allocated blocks
6. **Crash consistency**: After crash, file system is consistent

## Testing Strategy

### Unit Tests

Test individual components in isolation:
- Disk I/O operations
- Block allocator
- Inode operations
- Directory operations
- Log operations

### Integration Tests

Test component interactions:
- File creation and deletion
- Directory operations
- File read/write
- Transaction commit

### Fault Injection

Test error handling:
- Disk corruption (bit flips)
- Power loss (crash during operation)
- Disk full scenarios

### Fuzzing

Test with random operations:
- Random file operations
- Random crash points
- Invariant checking after each operation

## Performance Considerations

This is an educational implementation, not optimized for performance. However, basic optimizations include:

1. **Buffer cache**: Reduce disk I/O
2. **Batch operations**: Group related operations
3. **Lazy allocation**: Allocate blocks on demand

## Limitations

1. **Single-threaded**: No concurrent access
2. **No permissions**: No user/group/permissions
3. **No symbolic links**: Only hard links
4. **Small files**: Max 4MB per file
5. **Fixed size**: File system size set at creation

## Future Enhancements

Possible extensions for learning:

1. **Extent-based allocation**: Replace block lists with extents
2. **B-tree directories**: Scale to large directories
3. **Copy-on-write**: Alternative to journaling
4. **Compression**: Transparent compression
5. **Encryption**: File-level encryption

## References

1. **xv6 File System**: https://pdos.csail.mit.edu/6.828/2018/xv6/book-rev11.pdf
2. **ext2 Journaling**: Tweedie, S. (1998). "Journaling the Linux ext2fs Filesystem"
3. **OSTEP**: Arpaci-Dusseau, R. & A. "Operating Systems: Three Easy Pieces"

## Revision History

- 2026-01-24: Initial design document (Phase 1 complete)
