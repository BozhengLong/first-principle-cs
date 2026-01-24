# PROJECTS - 核心系统仓库清单

本文档列出所有计划中的系统仓库，包括状态、目标、关键不变量等信息。

## 仓库列表

| 仓库名 | 状态 | 一句话目标 | 入口命令 | 关键不变量 | 关联课程模块 |
|--------|------|-----------|---------|-----------|-------------|
| tiny-interpreter | active | 最小解释器，理解求值与作用域 | `python -m src.tiny_interpreter.main examples/factorial.lisp` | 词法作用域、闭包捕获 | Module B |
| simple-compiler | active | 简单编译器，理解代码生成 | `python -m src.simple_compiler.main examples/gcd.sl` | 类型安全、内存安全 | Module B |
| mini-os | active | 最小操作系统内核，理解进程与内存 | `make qemu` | 进程隔离、内存保护 | Module C |
| simple-fs | active | 简单文件系统，理解持久化与一致性 | `./mkfs disk.img` | 崩溃一致性、原子性 | Module C |
| storage-engine | planned | 存储引擎，理解索引与持久化 | `./storage-bench` | 持久化、有序性 | Module D |
| tx-manager | planned | 事务管理器，理解并发控制 | `./tx-test` | ACID 性质 | Module D |
| consensus | planned | 共识协议，理解分布式一致性 | `./raft-node` | 安全性、活性 | Module E |
| dist-kv | planned | 分布式键值存储，理解容错 | `./kv-server` | 线性一致性 | Module E |

## 详细说明

### tiny-interpreter

**状态**：✅ Active - [GitHub 仓库](https://github.com/first-principles-cs/tiny-interpreter)

**目标**：实现一个最小的 Lisp 风格解释器，理解程序如何被求值。

**核心特性**：
- 词法分析与语法分析
- 环境模型与闭包
- 递归支持

**关键不变量**：
- 词法作用域：变量查找遵循静态作用域规则
- 闭包捕获：闭包捕获定义时的环境

**验证方法**：
- 单元测试：43 个测试全部通过
- 集成测试：factorial（递归）、closure（闭包）示例
- 代码覆盖率：核心组件 100% 覆盖

**快速开始**：
```bash
git clone https://github.com/first-principles-cs/tiny-interpreter.git
cd tiny-interpreter
python3 -m src.tiny_interpreter.main examples/factorial.lisp
```

**关联课程**：Module B - 语言与执行

---

### simple-compiler

**状态**：✅ Active - [GitHub 仓库](https://github.com/first-principles-cs/simple-compiler)

**目标**：实现一个简单的编译器，将SimpleLang（类C语言）编译到栈式字节码并在虚拟机上执行。

**核心特性**：
- 词法分析与语法分析
- 静态类型检查
- 三地址码中间表示
- 栈式字节码生成
- 虚拟机执行

**关键不变量**：
- 类型安全：类型正确的程序不会在运行时产生类型错误
- 内存安全：基于栈的执行防止内存错误
- 语义保持：编译后的程序与源语义行为一致

**验证方法**：
- 单元测试：75个测试覆盖所有编译阶段
  - Lexer: 15个测试
  - Parser: 19个测试
  - Type Checker: 26个测试
  - 集成测试: 15个测试
- 示例程序：6个SimpleLang程序验证功能
- 代码统计：~1982行源代码，~882行测试代码

**快速开始**：
```bash
git clone https://github.com/first-principles-cs/simple-compiler.git
cd simple-compiler
python -m src.simple_compiler.main examples/gcd.sl
```

**关联课程**：Module B - 语言与执行

---

### mini-os

**状态**：✅ Active - [GitHub 仓库](https://github.com/first-principles-cs/mini-os)

**目标**：实现一个最小的操作系统内核，理解进程、内存、系统调用。

**核心特性**：
- 进程调度 (round-robin)
- 虚拟内存 (two-level page tables)
- 系统调用接口 (fork, exit, wait, etc.)
- 上下文切换
- 中断处理

**关键不变量**：
- 进程隔离：进程不能访问其他进程的内存
- 内存保护：用户态不能访问内核态内存
- 调度公平性：所有就绪进程最终会被调度

**验证方法**：
- 单元测试：35个文件，~2000行代码
- 调度器测试：验证公平性和状态转换
- 内存测试：验证隔离和保护
- 故障注入：测试进程崩溃场景

**快速开始**：
```bash
git clone https://github.com/first-principles-cs/mini-os.git
cd mini-os
make
make qemu
```

**关联课程**：Module C - 单机系统

---

### simple-fs

**状态**：✅ Active - [GitHub 仓库](https://github.com/first-principles-cs/simple-fs)

**目标**：实现一个简单的 Unix-like 文件系统，基于 xv6 设计，理解持久化与崩溃一致性。

**核心特性**：
- 磁盘 I/O 模拟层（Phase 1 ✅）
- 块分配器与位图管理（Phase 1 ✅）
- Inode 管理与缓存（Phase 2 ✅）
- 目录操作与路径解析（Phase 2 ✅）
- 文件创建、删除、读写（Phase 2 ✅）
- 缓冲区缓存（Phase 3 - 计划中）
- 日志或写时复制（Phase 4 - 计划中）

**关键不变量**：
- 崩溃一致性：崩溃后文件系统仍然可用
- 原子性：操作要么全部完成，要么全部不完成
- 引用计数正确性：inode 引用计数与实际引用匹配
- 块分配一致性：已分配块不会被重复分配

**验证方法**：
- 单元测试：12 个测试全部通过
  - Phase 1: 3 个测试（磁盘初始化、读写、块分配）
  - Phase 2: 9 个测试（inode 操作、I/O、目录、路径、文件）
- 集成测试：文件系统创建、文件操作、目录遍历
- 代码统计：~2570 行源代码和测试代码

**快速开始**：
```bash
git clone https://github.com/first-principles-cs/simple-fs.git
cd simple-fs
make
./mkfs fs.img
make test
```

**关联课程**：Module C - 单机系统

---

### storage-engine

**目标**：实现一个存储引擎，理解索引结构与持久化。

**核心特性**：
- LSM-tree 或 B-tree
- 压缩与合并
- 预写日志

**关键不变量**：
- 持久化：写入的数据不会丢失
- 有序性：范围查询返回有序结果

**验证方法**：
- 单元测试：测试索引操作
- 基准测试：对比不同索引结构的性能
- 故障注入：测试崩溃恢复

**关联课程**：Module D - 数据系统

---

### tx-manager

**目标**：实现一个事务管理器，理解并发控制与隔离级别。

**核心特性**：
- MVCC 或 2PL
- 死锁检测
- 隔离级别

**关键不变量**：
- 原子性：事务要么全部提交，要么全部回滚
- 隔离性：并发事务不会相互干扰
- 一致性：事务保持数据库约束
- 持久性：提交的事务不会丢失

**验证方法**：
- 单元测试：测试事务操作
- 并发测试：测试多线程场景
- Jepsen 风格测试：验证隔离级别

**关联课程**：Module D - 数据系统

---

### consensus

**目标**：实现一个共识协议（Raft 或 Paxos），理解分布式一致性。

**核心特性**：
- 领导者选举
- 日志复制
- 成员变更

**关键不变量**：
- 安全性：已提交的日志不会丢失
- 活性：系统最终会达成共识

**验证方法**：
- 单元测试：测试协议逻辑
- 网络分区测试：模拟网络故障
- Jepsen 测试：验证一致性

**关联课程**：Module E - 网络与分布式

---

### dist-kv

**目标**：实现一个分布式键值存储，理解分片、复制、容错。

**核心特性**：
- 一致性哈希
- 主从复制
- 故障检测与转移

**关键不变量**：
- 线性一致性或最终一致性
- 可用性：f < n/2 个节点故障时仍可用

**验证方法**：
- 单元测试：测试分片与复制逻辑
- 故障注入：测试节点崩溃、网络分区
- Jepsen 测试：验证一致性模型
- 基准测试：测量吞吐量与延迟

**关联课程**：Module E - 网络与分布式

---

## 状态说明

- **planned**: 计划中，尚未开始
- **active**: 正在开发
- **stable**: 已完成核心功能，进入维护阶段

## 更新记录

- 2025-01: 初始版本，所有项目处于 planned 状态

---

下一步：查看 [ROADMAP.md](./ROADMAP.md) 了解开发时间线。
