# Tiny Interpreter

> 从零实现一个编程语言解释器，理解程序是如何被"读懂"和"执行"的。

## 你将学到什么

完成这个项目后，你将能够回答：

- **程序是如何被"读懂"的？** （词法分析 + 语法分析）
- **变量是如何被"记住"的？** （环境模型）
- **函数是如何"工作"的？** （求值规则）
- **闭包是如何"捕获"环境的？** （词法作用域）

---

## 开始学习

### 方式一：交互式学习（推荐）

这是一个**学习产品**，不只是代码。我们设计了渐进式的学习路径：

```bash
# 1. 克隆仓库
git clone https://github.com/first-principles-cs/tiny-interpreter.git
cd tiny-interpreter

# 2. 安装依赖
pip install pytest

# 3. 开始学习之旅
cd learn/00-introduction
python playground.py
```

👉 **[点击这里查看完整学习指南](docs/LEARNING_GUIDE.md)**

### 方式二：直接查看实现

如果你想直接查看完整实现，代码在 `src/` 目录：

```bash
# 运行 REPL
python -m src.tiny_interpreter.main

# 运行示例
python -m src.tiny_interpreter.main examples/factorial.lisp

# 运行测试
pytest tests/ -v
```

---

## 学习路径

```
模块 0: 引言 ─────────── "为什么要学解释器？"
    ↓
模块 1: 词法分析 ─────── "如何把字符串变成有意义的单元？"
    ↓
模块 2: 语法分析 ─────── "如何把 Token 组织成树状结构？"
    ↓
模块 3: 环境模型 ─────── "变量的值存在哪里？"
    ↓
模块 4: 基础求值 ─────── "如何计算表达式的值？"
    ↓
模块 5: Lambda 与闭包 ── "函数如何记住它的环境？"（顿悟时刻！）
    ↓
模块 6: 整合与扩展 ───── "如何扩展语言？"
```

---

## 项目结构

```
tiny-interpreter/
├── learn/                  # 学习模块（推荐从这里开始）
│   ├── 00-introduction/   # 引言与动机
│   ├── 01-lexer/          # 词法分析
│   ├── 02-parser/         # 语法分析
│   ├── 03-environment/    # 环境模型
│   ├── 04-evaluator-basic/# 基础求值
│   ├── 05-evaluator-lambda/# Lambda 与闭包
│   └── 06-putting-together/# 整合与扩展
│
├── src/tiny_interpreter/   # 完整参考实现
├── tests/                  # 测试套件
├── examples/               # 示例程序
├── tools/                  # 学习辅助工具
└── docs/                   # 文档
```

---

## 语言特性

Tiny Interpreter 实现了一个 Lisp 风格的语言：

```lisp
; 定义变量
(define x 42)

; 定义函数
(define square (lambda (x) (* x x)))

; 条件表达式
(if (< x 10) "small" "large")

; 闭包
(define make-adder
  (lambda (x)
    (lambda (y) (+ x y))))

(define add5 (make-adder 5))
(add5 3)  ; => 8
```

---

## 辅助工具

```bash
# 检查学习进度
python tools/check_progress.py

# AST 可视化
python tools/visualize_ast.py "(+ 1 (* 2 3))"
```

---

## 核心接口

```python
# 词法分析
lexer = Lexer(source_code)
tokens = lexer.tokenize()

# 语法分析
parser = Parser(tokens)
ast = parser.parse()

# 求值
evaluator = Evaluator()
result = evaluator.run(source_code)
```

---

## 关键不变量

1. **词法作用域**：变量查找遵循静态作用域规则
2. **闭包捕获**：闭包捕获定义时的环境，而不是调用时的环境
3. **括号匹配**：所有左括号必须有对应的右括号
4. **类型安全**：运算符只能应用于正确类型的操作数

---

## 设计文档

详细的技术设计请参考 [docs/design.md](docs/design.md)。

---

## 许可证

MIT License - 详见 [LICENSE](LICENSE)

---

## 关联课程

本项目是 [first-principles-cs](https://github.com/first-principles-cs/guide) 课程体系的一部分。
