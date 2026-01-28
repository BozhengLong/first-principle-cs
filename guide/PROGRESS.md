# 项目进度记录

**最后更新**：2026-01-28

## 项目概述

first-principles-cs 是一个基于第一性原理重建计算机科学基础设施的项目。目标是在 AI 时代重新定义"理解"，通过从零实现核心系统来深入理解计算机科学。

**核心理念**：
> "What I cannot create, I do not understand." — Richard Feynman

**四大原则**：
1. 接口与不变量 - 明确系统边界，定义必须始终成立的性质
2. 失败模型 - 识别所有可能的失败场景，设计容错机制
3. 验证优先 - 测试先行，用属性测试和故障注入验证不变量
4. AI 协作协议 - 规格先行 → 生成实现 → 约束测试 → 重构压复杂度

## 已完成的工作

### 1. Guide 仓库（组织文档中心）

**仓库地址**：https://github.com/first-principles-cs/guide

**创建时间**：2026-01-22

**文件统计**：
- 22 个文件
- 3500+ 行文档
- 完整的文档结构

**包含内容**：

#### 核心文档（guide/）
- `WHY.md` - 项目动机与"理解"的定义
  - 三层理解级别：会用、会诊断、会创造抽象
  - 非目标：不追求刷题覆盖率、不复刻传统教材
- `ROADMAP.md` - 12 个月开发路线图
  - Q1: 基础与语言执行（tiny-interpreter, simple-compiler）
  - Q2: 单机系统（mini-os, simple-fs）
  - Q3: 数据系统（storage-engine, tx-manager）
  - Q4: 网络与分布式（consensus, dist-kv）
- `PROJECTS.md` - 8 个核心系统仓库清单
  - 当前状态：1 个 active，7 个 planned

#### 原则文档（principles/）
- `interface-and-invariants.md` - 接口与不变量（3 个跨领域例子）
- `failure-model.md` - 10 种失败模型清单
- `verification-first.md` - 测试金字塔、属性测试、Fuzz 测试
- `ai-collaboration-protocol.md` - 四阶段协作流程

#### 模板文件（templates/）
- `repo-readme-template.md` - 统一仓库 README 结构
- `design-doc-template.md` - 设计文档模板
- `experiment-template.md` - 实验模板

#### 课程体系（courses/）
- `module-a-foundations.md` - 基础（计算模型、抽象、复杂度）
- `module-b-language-execution.md` - 语言与执行（解释器、编译器）
- `module-c-single-node-systems.md` - 单机系统（操作系统、文件系统）
- `module-d-data-systems.md` - 数据系统（数据库、存储引擎）
- `module-e-network-distributed.md` - 网络与分布式（协议、一致性）
- `module-f-ai-search-systems.md` - AI 与搜索（搜索引擎、推荐系统）

### 2. Tiny Interpreter 仓库（第一个系统）

**仓库地址**：https://github.com/first-principles-cs/tiny-interpreter

**创建时间**：2026-01-22

**状态**：✅ Active

**文件统计**：
- 21 个文件
- 1700+ 行代码
- 43 个测试（全部通过）

**技术栈**：
- 语言：Python 3.10+
- 测试框架：pytest
- 代码覆盖率：核心组件 100%

**实现的组件**：

1. **词法分析器（Lexer）** - 180 行
   - Token 类型：LPAREN, RPAREN, NUMBER, SYMBOL, BOOLEAN
   - 位置跟踪（行号、列号）
   - 注释支持

2. **语法分析器（Parser）** - 150 行
   - AST 节点：Number, Boolean, Symbol, SExpression
   - 括号匹配验证
   - 错误处理与位置信息

3. **环境模型（Environment）** - 80 行
   - 词法作用域支持
   - 嵌套环境
   - 变量定义与查找

4. **求值器（Evaluator）** - 220 行
   - 特殊形式：define, lambda, if, quote, begin
   - 内置函数：算术（+, -, *, /）、比较（=, <, >）、列表操作（cons, car, cdr）
   - 闭包支持
   - 递归支持

**语言特性**：
- 数据类型：整数、布尔值、符号、列表
- 支持递归（如 factorial）
- 支持闭包（如 make-adder）
- 支持高阶函数

**测试覆盖**：
- 词法分析器：20 个测试
- 语法分析器：10 个测试
- 求值器：15 个测试
- 集成测试：4 个测试

**示例程序**：
- `hello.lisp` - Hello World
- `factorial.lisp` - 递归阶乘
- `closure.lisp` - 闭包示例

**验证结果**:
```
============================= 43 passed in 0.05s ==============================
```

### 3. Mini-OS 仓库（第三个系统）

**仓库地址**：https://github.com/first-principles-cs/mini-os

**创建时间**：2026-01-24

**状态**：✅ Active

**文件统计**：
- 35 个文件
- 2000+ 行代码
- 7 个测试框架

**技术栈**：
- 语言：C (kernel), x86 Assembly
- 平台：x86 (32-bit)
- 模拟器：QEMU
- 工具链：GCC, GNU AS, LD

**实现的组件**：

1. **进程管理（proc.c）** - 378 行
   - 进程表管理
   - 进程调度器（round-robin）
   - fork/exit/wait 系统调用
   - 上下文切换（swtch.S）
   - CPU 状态管理

2. **内存管理（vm.c）** - 250 行
   - 两级页表（page directory + page table）
   - 虚拟内存映射（mappages, walkpgdir）
   - 内核内存初始化
   - 物理内存分配器（kalloc.c）

3. **系统调用（syscall.c）** - 180 行
   - 系统调用接口
   - 21 个系统调用定义
   - 参数传递机制

4. **中断处理（trap.c, trapasm.S）** - 150 行
   - IDT 初始化
   - 256 个中断向量（vectors.S）
   - Trap frame 管理
   - 系统调用入口

5. **同步原语（spinlock.c）** - 80 行
   - Spinlock 实现
   - 中断禁用/启用
   - 死锁检测

**核心特性**：
- 进程隔离：每个进程有独立的页表
- 内存保护：用户态不能访问内核态内存
- 调度公平性：Round-robin 调度算法
- 上下文切换：保存/恢复寄存器状态

**关键不变量**：
- 进程隔离不变量：进程不能访问其他进程的内存
- 内存保护不变量：用户态不能访问内核态内存
- 调度公平性不变量：所有就绪进程最终会被调度

**快速开始**：
```bash
git clone https://github.com/first-principles-cs/mini-os.git
cd mini-os
make
make qemu
```

### 4. Simple-FS 仓库（简单文件系统）

**仓库地址**：https://github.com/first-principles-cs/simple-fs

**创建时间**：2026-01-24

**状态**：✅ Active

**文件统计**：
- 25 个文件
- 2570 行代码
- 12 个测试（全部通过）

**技术栈**：
- 语言：C
- 构建系统：Makefile
- 测试框架：自定义单元测试
- 设计基础：xv6 文件系统

**实现的组件**：

**Phase 1: 磁盘 I/O 和块分配（已完成 ✅）**

1. **磁盘 I/O 层（disk.c）** - 112 行
   - 块级读写操作
   - 磁盘镜像创建和管理
   - 4KB 块大小
   - 断言保护

2. **块分配器（block.c）** - 79 行
   - 位图管理
   - balloc/bfree 操作
   - 块零化
   - 持久化支持

3. **mkfs 工具（mkfs.c）** - 245 行
   - 文件系统格式化
   - 超级块初始化
   - 根目录创建
   - Inode 和位图初始化

**Phase 2: Inode 和目录操作（已完成 ✅）**

1. **锁基础设施（lock.c）** - 28 行
   - pthread_mutex 封装
   - Spinlock 接口
   - 锁获取/释放

2. **Inode 管理（inode.c）** - 338 行
   - 内存 inode 缓存（50 个条目）
   - 引用计数
   - Inode 分配/释放
   - 块映射（直接块 + 间接块）
   - 文件 I/O（read/write）
   - Inode 截断

3. **目录操作（dir.c）** - 106 行
   - 目录查找
   - 添加/删除目录项
   - 目录空检查
   - 线性扫描实现

4. **路径解析（namei.c）** - 92 行
   - 绝对路径支持
   - 路径组件解析
   - 父目录查找
   - 迭代实现

5. **文件操作（file.c）** - 118 行
   - 文件/目录创建
   - 文件/目录删除
   - 链接管理
   - "." 和 ".." 支持

**Phase 3: 缓冲区缓存（已完成 ✅）**

6. **缓冲区缓存（buf.c）** - 161 行
   - LRU 淘汰策略
   - 引用计数防止淘汰
   - 写穿透策略
   - 缓存命中优化

**Phase 4: 日志系统（已完成 ✅）**

7. **写前日志（log.c）** - 218 行
   - 事务接口（begin_op/end_op）
   - 日志头管理
   - 崩溃恢复
   - 组提交优化

**Phase 5: 交互式 Shell（已完成 ✅）**

8. **Shell（shell.c）** - 400+ 行
   - 命令解析器
   - 文件系统命令（ls, mkdir, touch, rm, cat, write, append, stat）
   - 导航命令（pwd, cd）
   - 路径规范化（处理 . 和 ..）

**核心特性**：
- 基于 xv6 设计的简单文件系统
- 支持直接块（0-11）和间接块（12）
- 最大文件大小：~4MB
- 线程安全的 inode 操作
- 持久化到磁盘镜像

**关键不变量**：
- 引用计数正确性：inode 引用计数与实际引用匹配
- 块分配一致性：已分配块不会被重复分配
- 目录一致性：目录项与 inode 状态一致
- 锁顺序：避免死锁的锁获取顺序

**测试覆盖**：
- Phase 1 测试（3 个）：
  - 磁盘初始化
  - 磁盘读写
  - 块分配器
- Phase 2 测试（9 个）：
  - Inode 分配
  - Inode 缓存
  - Inode I/O
  - 块映射
  - 目录操作
  - 路径解析
  - 文件创建
  - 文件删除
  - 集成测试
- Phase 3 测试（6 个）：
  - bread/brelse
  - 缓存命中
  - 写穿透
  - LRU 淘汰
  - 引用计数防淘汰
  - Phase 2 回归
- Phase 4 测试（5 个）：
  - 基本事务
  - 多操作事务
  - 日志头清理
  - 带事务的 unlink
  - Phase 3 回归
- Phase 5 测试（8 个）：
  - 路径规范化
  - mkdir/touch 基本操作
  - write 和 cat
  - ls 目录列表
  - rm 文件删除
  - 类型字符串转换
  - Phase 4 回归

**快速开始**：
```bash
git clone https://github.com/first-principles-cs/simple-fs.git
cd simple-fs
make
./mkfs fs.img
make test
./shell fs.img  # Interactive shell
```

**项目状态**：✅ 全部 5 个阶段已完成

### 5. Storage-Engine 仓库（存储引擎）

**仓库地址**：https://github.com/first-principles-cs/storage-engine

**创建时间**：2026-01-27

**状态**：✅ Active (Phase 1-3 完成)

**文件统计**：
- 19 个文件
- 2800+ 行代码
- 30 个测试（全部通过）

**技术栈**：
- 语言：C
- 构建系统：Makefile
- 测试框架：自定义单元测试
- 数据结构：LSM-tree

**实现的组件**：

**Phase 1: 基础设施（已完成 ✅）**

1. **Skip List（skiplist.c）** - 360 行
   - O(log n) 插入、查找、删除
   - 随机层级生成
   - 迭代器支持
   - 内存使用跟踪

2. **MemTable（memtable.c）** - 100 行
   - Skip List 封装
   - 大小限制检测
   - 迭代器接口

3. **Storage API（storage.c）** - 170 行
   - put/get/delete 操作
   - 迭代器（自动跳过删除项）
   - 内存统计

**Phase 2: 写前日志 WAL（已完成 ✅）**

4. **CRC32（crc32.c）** - 50 行
   - 查表法实现
   - 数据完整性校验

5. **WAL（wal.c）** - 200 行
   - 记录格式：CRC32 + 类型 + 长度 + 数据
   - 同步写入选项
   - 崩溃恢复

**Phase 3: SSTable（已完成 ✅）**

6. **Bloom Filter（bloom.c）** - 100 行
   - MurmurHash3 哈希
   - Double hashing（7 个哈希函数）
   - 序列化/反序列化

7. **SSTable（sstable.c）** - 400 行
   - 前缀压缩
   - Restart points（每 16 个 key）
   - 二分查找索引
   - CRC32 校验

**待实现阶段**：
- Phase 4: 多层 LSM 与 Compaction
- Phase 5: Block Cache 与基准测试

**核心特性**：
- 基于 LSM-tree 的键值存储
- Skip List 内存索引
- WAL 崩溃恢复
- SSTable 磁盘持久化
- Bloom Filter 快速过滤
- 有序迭代器
- 删除标记（tombstone）

**关键不变量**：
- 持久化：写入的数据不会丢失（Phase 2+）
- 有序性：范围查询返回有序结果
- 一致性：Compaction 不丢失数据（Phase 4+）
- 原子性：操作要么全部完成，要么全部不完成

**测试覆盖**：
- Phase 1 测试（12 个）：
  - Skip List 测试（5 个）：创建/销毁、put/get、更新、删除、迭代器
  - MemTable 测试（2 个）：基本操作、flush 阈值检测
  - Storage API 测试（5 个）：open/close、put/get、delete、迭代器、跳过删除项
- Phase 2 测试（8 个）：
  - CRC32 测试（2 个）：基本校验、空数据
  - WAL 测试（6 个）：写入/读取、恢复、损坏检测、同步模式
- Phase 3 测试（10 个）：
  - Bloom Filter 测试（3 个）：基本操作、假阳性率、序列化
  - SSTable 测试（4 个）：写入/读取、多 block、tombstone、未找到
  - Storage 集成测试（3 个）：flush、flush 后查询、多次 flush

**快速开始**：
```bash
git clone https://github.com/first-principles-cs/storage-engine.git
cd storage-engine
make test
```

**项目状态**：Phase 1-3 完成，Phase 4-5 计划中

## 当前状态

### 完成度

| 阶段 | 状态 | 完成时间 |
|------|------|---------|
| 项目规划 | ✅ 完成 | 2026-01-22 |
| Guide 仓库 | ✅ 完成 | 2026-01-22 |
| Q1 项目 1: tiny-interpreter | ✅ 完成 | 2026-01-22 |
| Q1 项目 2: simple-compiler | ✅ 完成 | 2026-01-22 |
| Q2 项目 1: mini-os | ✅ 完成 | 2026-01-24 |
| Q2 项目 2: simple-fs (All Phases) | ✅ 完成 | 2026-01-27 |
| Q3 项目 1: storage-engine (Phase 1) | ✅ 完成 | 2026-01-27 |
| Q3 项目 1: storage-engine (Phase 2) | ✅ 完成 | 2026-01-28 |
| Q3 项目 1: storage-engine (Phase 3) | ✅ 完成 | 2026-01-28 |

### 仓库状态

| 仓库名 | 状态 | GitHub 链接 | 文件数 | 代码行数 | 测试数 |
|--------|------|------------|--------|---------|--------|
| guide | ✅ Active | [链接](https://github.com/first-principles-cs/guide) | 22 | 3500+ | - |
| tiny-interpreter | ✅ Active | [链接](https://github.com/first-principles-cs/tiny-interpreter) | 21 | 1700+ | 43 |
| simple-compiler | ✅ Active | [链接](https://github.com/first-principles-cs/simple-compiler) | 32 | 1982 | 75 |
| mini-os | ✅ Active | [链接](https://github.com/first-principles-cs/mini-os) | 35 | 2000+ | 7 |
| simple-fs | ✅ Complete | [链接](https://github.com/first-principles-cs/simple-fs) | 30+ | 3500+ | 31 |
| storage-engine | ✅ Active | [链接](https://github.com/first-principles-cs/storage-engine) | 19 | 2800+ | 30 |
| tx-manager | 📋 Planned | - | - | - | - |
| consensus | 📋 Planned | - | - | - | - |
| dist-kv | 📋 Planned | - | - | - | - |

### 累计统计

- **总仓库数**：6 个（1 个文档仓库 + 5 个系统仓库）
- **总文件数**：160+ 个
- **总代码行数**：15500+ 行
- **总测试数**：186 个
- **测试通过率**：100%

## 技术决策记录

### 1. 编程语言选择

**Tiny Interpreter**：Python 3.10+
- **理由**：简洁易读，适合教学；快速原型开发；丰富的测试工具
- **权衡**：性能不是最优，但清晰度优先

### 2. 语言设计选择

**Tiny Interpreter**：Lisp 风格
- **理由**：语法简单，易于解析；S-表达式统一代码和数据；适合教学
- **权衡**：不是主流语法，但概念清晰

### 3. 实现策略

**解释执行 vs 编译**：
- Tiny Interpreter 采用解释执行
- Simple Compiler 将采用编译到字节码
- **理由**：分别理解两种不同的执行模型

### 4. 测试策略

**测试金字塔**：
- 大量单元测试（80%）
- 中等集成测试（15%）
- 少量端到端测试（5%）
- **理由**：快速反馈，精确定位问题

## 下一步计划

### 立即任务：创建 simple-compiler

**目标**：实现一个简单的编译器，将高级语言编译到字节码或汇编

**技术选择**：
- 编程语言：待定（可能是 Python 或 Rust）
- 目标语言：字节码或简单的汇编
- 源语言：简单的类 C 语言

**核心组件**：
1. 词法分析器
2. 语法分析器
3. 类型检查器
4. 中间表示（IR）
5. 代码生成器
6. 寄存器分配

**预计工作量**：
- 文件数：~25 个
- 代码行数：~2500 行
- 测试数：~50 个

### Q1 剩余任务

- [ ] 完成 simple-compiler
- [ ] 更新 guide/PROJECTS.md
- [ ] 编写 Q1 总结报告

### Q2 规划

- [ ] mini-os：最小操作系统内核
- [ ] simple-fs：简单文件系统

## 重要链接

### GitHub Organization
- **Organization 主页**：https://github.com/first-principles-cs
- **Guide 仓库**：https://github.com/first-principles-cs/guide
- **Tiny Interpreter**：https://github.com/first-principles-cs/tiny-interpreter

### 文档
- **WHY**：https://github.com/first-principles-cs/guide/blob/main/guide/WHY.md
- **ROADMAP**：https://github.com/first-principles-cs/guide/blob/main/guide/ROADMAP.md
- **PROJECTS**：https://github.com/first-principles-cs/guide/blob/main/guide/PROJECTS.md

### 课程
- **Module B**：https://github.com/first-principles-cs/guide/blob/main/courses/module-b-language-execution.md

## 学习成果

### 通过 Tiny Interpreter 学到的内容

1. **词法分析**：
   - 如何将字符流转换为 token 流
   - 位置跟踪的重要性
   - 错误处理策略

2. **语法分析**：
   - 递归下降解析
   - AST 的设计
   - 括号匹配验证

3. **环境模型**：
   - 词法作用域的实现
   - 环境链的工作原理
   - 变量查找算法

4. **求值**：
   - 特殊形式 vs 函数调用
   - 闭包的实现
   - 递归的支持

5. **测试**：
   - 单元测试的重要性
   - 集成测试的价值
   - 测试驱动开发

## 待办事项

### 短期（本周）
- [ ] 创建 simple-compiler 仓库
- [ ] 实现编译器核心组件
- [ ] 编写完整测试套件

### 中期（本月）
- [ ] 完成 Q1 的两个项目
- [ ] 编写 Q1 总结报告
- [ ] 规划 Q2 项目

### 长期（本季度）
- [ ] 完成 Module B 的所有项目
- [ ] 开始 Module C 的规划
- [ ] 建立社区（如果需要）

## 注意事项

1. **保持简单**：这是"最小"实现，不要过度设计
2. **测试先行**：每个组件都要有测试
3. **文档同步**：代码和文档要保持一致
4. **遵循原则**：应用四大原则（接口与不变量、失败模型、验证优先、AI 协作协议）
5. **记录决策**：重要的技术决策要记录在案

## 更新日志

- **2026-01-22**：
  - 创建 guide 仓库，包含完整的文档结构
  - 创建 tiny-interpreter 仓库，实现完整的解释器
  - 更新 PROJECTS.md，标记 tiny-interpreter 为 active
  - 创建本进度记录文档
