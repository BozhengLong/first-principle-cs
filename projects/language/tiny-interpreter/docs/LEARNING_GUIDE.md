# Tiny Interpreter 学习指南

> 从零实现一个编程语言解释器，理解程序是如何被"读懂"和"执行"的。

## 学习目标

完成这个项目后，你将能够回答：

| 问题 | 你将学到 |
|------|---------|
| 程序是如何被"读懂"的？ | 词法分析 + 语法分析 |
| 变量是如何被"记住"的？ | 环境模型 |
| 表达式是如何被"计算"的？ | 求值规则 |
| 闭包是如何"捕获"环境的？ | 词法作用域 |

---

## 学习路径

```
模块 0: 引言
    "为什么要学解释器？"
    ↓
模块 1: 词法分析
    "如何把字符串变成有意义的单元？"
    ↓
模块 2: 语法分析
    "如何把 Token 组织成树状结构？"
    ↓
模块 3: 环境模型
    "变量的值存在哪里？"
    ↓
模块 4: 基础求值
    "如何计算表达式的值？"
    ↓
模块 5: Lambda 与闭包
    "函数如何记住它的环境？"（顿悟时刻！）
    ↓
模块 6: 整合与扩展
    "如何扩展语言？"
```

---

## 每个模块的结构

每个学习模块包含：

| 文件 | 说明 |
|------|------|
| `README.md` | 学习指南：问题引入、核心概念、动手指南 |
| `skeleton.py` | 骨架代码：你需要填充的实现 |
| `test_skeleton.py` | 测试：验证你的实现 |
| `challenge.md` | 挑战题：深入思考 |
| `hints/` | 渐进式提示：卡住时查看 |

---

## 学习方法

### 1. 问题驱动

每个模块从一个问题开始。先思考，再看答案。

### 2. 动手实现

不要只是阅读代码。打开 `skeleton.py`，自己实现！

### 3. 测试验证

```bash
cd learn/01-lexer
pytest test_skeleton.py -v
```

测试通过 = 理解正确。

### 4. 渐进提示

卡住了？查看 `hints/` 目录：
- `hint1.md` - 基本思路
- `hint2.md` - 更多细节
- `hint3.md` - 接近答案

### 5. 深入思考

完成实现后，阅读 `challenge.md` 中的挑战题。

---

## 快速开始

```bash
# 1. 进入项目目录
cd tiny-interpreter

# 2. 安装依赖
pip install pytest

# 3. 开始学习
cd learn/00-introduction
python playground.py

# 4. 检查进度
python tools/check_progress.py
```

---

## 辅助工具

### 检查学习进度

```bash
python tools/check_progress.py
```

### AST 可视化

```bash
python tools/visualize_ast.py "(+ 1 (* 2 3))"
```

### 运行完整解释器

```bash
python -m src.tiny_interpreter.main
```

---

## 预计时间

| 模块 | 预计时间 |
|------|---------|
| 模块 0: 引言 | 30 分钟 |
| 模块 1: 词法分析 | 1-2 小时 |
| 模块 2: 语法分析 | 1-2 小时 |
| 模块 3: 环境模型 | 1 小时 |
| 模块 4: 基础求值 | 1-2 小时 |
| 模块 5: Lambda 与闭包 | 2-3 小时 |
| 模块 6: 整合与扩展 | 自由探索 |

总计：约 8-12 小时

---

## 参考资源

### 书籍

- **SICP**：《计算机程序的构造和解释》
- **Crafting Interpreters**：现代解释器教程
- **PLAI**：《Programming Languages: Application and Interpretation》

### 在线资源

- [Crafting Interpreters](https://craftinginterpreters.com/)
- [SICP 在线版](https://mitpress.mit.edu/sites/default/files/sicp/full-text/book/book.html)

---

## 常见问题

### Q: 需要什么前置知识？

A: 基本的 Python 编程能力。不需要编译原理背景。

### Q: 卡住了怎么办？

A:
1. 查看 `hints/` 目录
2. 参考 `src/` 中的完整实现
3. 在 GitHub Issues 提问

### Q: 可以跳过某些模块吗？

A: 不建议。每个模块都依赖前面的概念。

### Q: 完成后可以做什么？

A:
1. 尝试 `challenge.md` 中的挑战
2. 实现 `extensions/` 中的扩展
3. 学习 `simple-compiler` 项目

---

## 开始学习

准备好了吗？

👉 **[点击这里开始学习之旅](../learn/00-introduction/README.md)**
