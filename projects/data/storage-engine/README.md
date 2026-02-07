[中文](README.md) | [English](README.en.md)

---

# storage-engine

LSM-tree 基础的键值存储引擎，理解索引结构与持久化。

## 项目状态

**Phase 1: 基础设施** ✅ 完成

- [x] Skip List 实现
- [x] MemTable 封装
- [x] 内存 put/get/delete
- [x] 单元测试 (12 个)

**Phase 2: 写前日志 WAL** ✅ 完成

- [x] WAL 记录格式
- [x] CRC32 校验
- [x] 崩溃恢复
- [x] 单元测试 (8 个)

**Phase 3: SSTable** ✅ 完成

- [x] Bloom Filter 实现
- [x] SSTable 写入器（前缀压缩、restart points）
- [x] SSTable 读取器（二分查找、CRC32 校验）
- [x] Storage 集成（flush、跨层查询）
- [x] 单元测试 (10 个)

**Phase 4: 多层 LSM** ✅ 完成

- [x] Level Manager 实现
- [x] L0 允许重叠文件
- [x] L1+ 按 min_key 排序
- [x] Compaction 触发检测
- [x] Manifest 持久化与恢复
- [x] 单元测试 (9 个)

**Phase 5: Block Cache 与基准测试** ✅ 完成

- [x] LRU Block Cache 实现
- [x] Hash table 加速查找
- [x] 缓存命中率统计
- [x] Benchmark 工具（顺序/随机读写、混合负载）
- [x] 单元测试 (8 个)

## 快速开始

```bash
# 编译
make

# 运行测试
make test

# 运行基准测试 (Phase 5 完成后)
./storage-bench
```

## 架构

```
+------------------+
|  storage-bench   |  (入口/CLI)
+--------+---------+
         |
+--------v---------+
|   Storage API    |  (公共接口)
+--------+---------+
         |
+--------+--------+--------+
|        |        |        |
v        v        v        v
MemTable SSTable  WAL    Cache
(SkipList)(磁盘)  (日志) (LRU)
```

## API

```c
// 生命周期
storage_t* storage_open(const char* path, storage_opts_t* opts);
void storage_close(storage_t* db);

// 基本操作
status_t storage_put(storage_t* db, const char* key, size_t key_len,
                     const char* val, size_t val_len);
status_t storage_get(storage_t* db, const char* key, size_t key_len,
                     char** val, size_t* val_len);
status_t storage_delete(storage_t* db, const char* key, size_t key_len);

// 范围操作
storage_iter_t* storage_iter_create(storage_t* db);
void storage_iter_seek(storage_iter_t* iter, const char* key, size_t key_len);
bool storage_iter_valid(storage_iter_t* iter);
void storage_iter_next(storage_iter_t* iter);
void storage_iter_destroy(storage_iter_t* iter);

// 维护
status_t storage_compact(storage_t* db);
status_t storage_flush(storage_t* db);
```

## 关键不变量

1. **持久化**: 写入的数据不会丢失
2. **有序性**: 范围查询返回有序结果
3. **一致性**: Compaction 不丢失数据
4. **原子性**: 操作要么全部完成，要么全部不完成

## 目录结构

```
storage-engine/
├── Makefile
├── README.md
├── docs/design.md
├── src/
│   ├── types.h, param.h      # 类型和参数
│   ├── storage.h/c           # 公共 API
│   ├── skiplist.h/c          # Skip List
│   ├── memtable.h/c          # MemTable
│   ├── wal.h/c, crc32.h/c    # WAL
│   ├── sstable.h/c           # SSTable
│   ├── bloom.h/c             # Bloom Filter
│   ├── level.h/c             # Level 管理
│   ├── compact.h/c           # Compaction
│   ├── cache.h/c             # Block Cache
│   └── bench.c               # 基准测试
└── tests/unit/
    └── test_phase[1-5].c
```

## 关联课程

Module D - 数据系统
