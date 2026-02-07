[中文](README.md) | [English](README.en.md)

---

# tx-manager

事务管理器，理解并发控制与隔离级别。

## 项目描述

tx-manager 是 [first-principles-cs](https://github.com/first-principles-cs) 项目的一部分，旨在通过从零实现一个事务管理器来深入理解数据库事务的核心概念。

## 核心特性

- MVCC（多版本并发控制）或 2PL（两阶段锁）
- 死锁检测
- 多种隔离级别（Read Committed、Repeatable Read、Serializable）

## 关键不变量

1. **原子性**：事务要么全部提交，要么全部回滚
2. **隔离性**：并发事务不会相互干扰
3. **一致性**：事务保持数据库约束
4. **持久性**：提交的事务不会丢失

## 快速开始

```bash
make
make test
./tx-test
```

## 验证方法

- 单元测试：测试事务操作
- 并发测试：测试多线程场景
- Jepsen 风格测试：验证隔离级别

## 关联课程

Module D - 数据系统
