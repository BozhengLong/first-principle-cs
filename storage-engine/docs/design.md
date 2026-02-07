# Storage Engine 设计文档

## 概述

本项目实现一个基于 LSM-tree 的键值存储引擎。LSM-tree (Log-Structured Merge-tree) 是一种针对写入优化的数据结构，广泛应用于 LevelDB、RocksDB、Cassandra 等系统。

## 为什么选择 LSM-tree

相比 B-tree，LSM-tree 有以下优势：

1. **实现更简单**: 追加写入，无需复杂的页分裂
2. **写入性能优秀**: 顺序写入磁盘，利用磁盘带宽
3. **组件分离清晰**: MemTable、SSTable、Compaction 各司其职
4. **与 simple-fs 衔接**: WAL 概念与 simple-fs 的日志系统相似

## 核心组件

### 1. MemTable (内存表)

- 基于 Skip List 实现
- 支持 O(log n) 的插入、查找、删除
- 达到阈值后 flush 到 SSTable

### 2. WAL (写前日志)

- 保证持久性
- 记录格式: `[length][crc32][key_len][key][val_len][val]`
- 崩溃恢复时重放日志

### 3. SSTable (排序字符串表)

- 不可变的磁盘文件
- 包含数据块、索引块、Bloom Filter
- 支持二分查找

### 4. Level Manager (层级管理)

- L0: 直接从 MemTable flush，可能有重叠
- L1+: 每层大小是上层的 10 倍，无重叠
- Leveled Compaction 策略

### 5. Block Cache (块缓存)

- LRU 淘汰策略
- 缓存热点数据块

## 数据流

### 写入流程

```
Put(key, value)
    |
    v
+-------+     +-------+
|  WAL  | --> | Disk  |  (持久化)
+-------+     +-------+
    |
    v
+-----------+
| MemTable  |  (内存)
+-----------+
    |
    v (达到阈值)
+-----------+
|  SSTable  |  (磁盘)
+-----------+
```

### 读取流程

```
Get(key)
    |
    v
+-----------+
| MemTable  |  (先查内存)
+-----------+
    | miss
    v
+-----------+
|   Cache   |  (查缓存)
+-----------+
    | miss
    v
+-----------+
| L0 SST    |  (查 L0，可能多个)
+-----------+
    | miss
    v
+-----------+
| L1+ SST   |  (查更深层级)
+-----------+
```

## 文件格式

### WAL 记录格式

```
+--------+--------+----------+-----+----------+-------+
| length | crc32  | key_len  | key | val_len  | value |
| 4B     | 4B     | 4B       | var | 4B       | var   |
+--------+--------+----------+-----+----------+-------+
```

### SSTable 文件格式

```
+------------------+
|   Data Block 0   |
+------------------+
|   Data Block 1   |
+------------------+
|       ...        |
+------------------+
|   Data Block N   |
+------------------+
|   Index Block    |
+------------------+
|   Bloom Filter   |
+------------------+
|     Footer       |
+------------------+
```

## 分阶段实现

### Phase 1: 基础设施
- Skip List 数据结构
- MemTable 封装
- 内存 put/get/delete

### Phase 2: WAL
- WAL 记录格式
- CRC32 校验
- 崩溃恢复

### Phase 3: SSTable
- SSTable 文件格式
- 写入器和读取器
- Bloom Filter

### Phase 4: 多层 LSM
- Level 管理
- Leveled Compaction
- Merge Iterator

### Phase 5: 集成
- 完整 Storage API
- Block Cache
- 基准测试工具
