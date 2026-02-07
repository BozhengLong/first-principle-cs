# simple-compiler

一个简单的编译器，将类 C 语言（SimpleLang）编译到基于栈的字节码。

## 项目描述

simple-compiler 是 [first-principles-cs](https://github.com/first-principles-cs) 项目的一部分，旨在通过从零实现一个编译器来深入理解编译原理。

**核心特性**：
- 静态类型检查
- 编译到基于栈的字节码
- 完整的虚拟机执行环境
- 支持函数、递归、循环等核心特性

## 快速开始

### 安装

```bash
pip install -r requirements.txt
```

### 编译并运行程序

```bash
# 编译 SimpleLang 程序
python -m src.simple_compiler.main examples/factorial.sl

# 运行生成的字节码
python -m src.simple_compiler.vm examples/factorial.bc
```

### 运行测试

```bash
make test
```

## SimpleLang 语言示例

```c
// 递归阶乘
int factorial(int n) {
    if (n <= 1) {
        return 1;
    } else {
        return n * factorial(n - 1);
    }
}

int main() {
    return factorial(5);  // 返回 120
}
```

## 架构概览

编译器由以下组件组成：

1. **Lexer（词法分析器）**：将源代码转换为 token 流
2. **Parser（语法分析器）**：将 token 流转换为抽象语法树（AST）
3. **Type Checker（类型检查器）**：验证类型安全
4. **IR Generator（中间表示生成器）**：将 AST 转换为三地址码
5. **Code Generator（代码生成器）**：将 IR 转换为字节码
6. **VM（虚拟机）**：执行字节码

## 文档

- [设计文档](./docs/design.md)
- [语言规范](./docs/language-spec.md)
- [字节码规范](./docs/bytecode-spec.md)

## 许可证

本项目采用 MIT License，详见 [LICENSE](./LICENSE)。

## 相关课程

本项目对应 [Module B - 语言与执行](https://github.com/first-principles-cs/guide/blob/main/courses/module-b-language-execution.md)。
