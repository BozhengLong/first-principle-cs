# Tiny Interpreter 设计文档

## 元信息

- **作者**：first-principles-cs
- **日期**：2025-01-22
- **状态**：已实现
- **版本**：v0.1.0

## 目标

实现一个最小的 Lisp 风格解释器，用于理解：
1. 程序如何被解析（词法分析 + 语法分析）
2. 程序如何被求值（环境模型 + 求值规则）
3. 闭包如何工作（词法作用域）

### 核心目标

1. 实现完整的解释器流程（lexing → parsing → evaluation）
2. 支持函数定义与调用
3. 支持词法作用域与闭包
4. 提供清晰的错误信息

### 非目标

1. **不实现尾调用优化** - 留待后续迭代
2. **不实现宏系统** - 超出最小实现范围
3. **不实现标准库** - 只实现核心特性
4. **不追求高性能** - 代码清晰度优先

## 接口定义

### 词法分析器（Lexer）

```python
class Lexer:
    def __init__(self, source: str)
    def tokenize(self) -> List[Token]
```

**语义**：将源代码字符串转换为 token 列表。

### 语法分析器（Parser）

```python
class Parser:
    def __init__(self, tokens: List[Token])
    def parse() -> List[ASTNode]
```

**语义**：将 token 列表转换为 AST 节点列表。

### 求值器（Evaluator）

```python
class Evaluator:
    def eval(self, node: ASTNode, env: Environment) -> Any
    def run(self, source: str) -> Any
```

**语义**：在给定环境中对 AST 节点求值。

## 关键不变量

### 1. 词法作用域

**描述**：变量查找遵循静态作用域规则。

**验证方法**：
```python
def test_lexical_scope():
    code = """
    (define x 1)
    (define f (lambda () x))
    (define x 2)
    (f)  ; 应该返回 1，不是 2
    """
```

### 2. 闭包捕获

**描述**：闭包捕获定义时的环境。

**验证方法**：测试闭包能否访问外层变量。

### 3. 括号匹配

**描述**：所有左括号必须有对应的右括号。

**验证方法**：Parser 在括号不匹配时抛出 ParserError。

## 失败模型

### 1. 词法错误

**场景**：非法字符、格式错误的 token

**应对**：抛出 LexerError，包含行号和列号

**测试**：
```python
def test_invalid_char():
    with pytest.raises(LexerError):
        Lexer("@").tokenize()
```

### 2. 语法错误

**场景**：括号不匹配

**应对**：抛出 ParserError

**测试**：
```python
def test_unmatched_paren():
    with pytest.raises(ParserError):
        parse("(+ 1 2")
```

### 3. 运行时错误

**场景**：未定义的变量、类型错误

**应对**：抛出 NameError 或 EvaluatorError

**测试**：
```python
def test_undefined_var():
    with pytest.raises(NameError):
        Evaluator().run("undefined")
```

## 设计取舍

### 取舍 1: Lisp vs 其他语法

**选择**：Lisp

**理由**：
- 语法简单，易于解析
- S-表达式统一了代码和数据
- 适合教学

### 取舍 2: 解释执行 vs 编译

**选择**：解释执行

**理由**：
- 实现简单
- 适合理解求值过程
- 编译留待 simple-compiler 项目

### 取舍 3: 动态类型 vs 静态类型

**选择**：动态类型

**理由**：
- 实现简单
- 符合 Lisp 传统
- 静态类型留待 simple-compiler 项目

## 验证计划

### 单元测试

- Lexer: 20+ 测试用例
- Parser: 10+ 测试用例
- Evaluator: 15+ 测试用例

### 集成测试

- 运行示例程序（factorial, closure）
- 验证输出正确

### 性能测试

- 不是当前重点，但应该能处理中等规模的程序

## 实施状态

- [x] 词法分析器
- [x] 语法分析器
- [x] 环境模型
- [x] 求值器
- [x] 单元测试
- [x] 集成测试
- [x] 示例程序
- [x] 文档

## 参考资料

- **《程序设计的抽象思维》（SICP）** - 第 4 章：元循环求值器
- **Crafting Interpreters** - Bob Nystrom
- **《编译原理》（Dragon Book）** - Aho et al.
