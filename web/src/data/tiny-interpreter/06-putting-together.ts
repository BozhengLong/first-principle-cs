import type { LearningModule } from "../types";

export const module06PuttingTogether: LearningModule = {
  id: "06-putting-together",
  index: 6,
  slug: "putting-together",
  hasCode: false,
  readme: {
    zh: `# 模块 6：整合与扩展

> "如何扩展语言？"

## 恭喜！

如果你完成了前面的模块，你已经实现了一个完整的解释器！

让我们回顾一下你学到了什么：

| 模块 | 你学到了 |
|------|---------|
| 词法分析 | 如何把字符串变成 Token |
| 语法分析 | 如何把 Token 变成 AST |
| 环境模型 | 如何存储和查找变量 |
| 基础求值 | 如何执行表达式 |
| Lambda 与闭包 | 函数如何"记住"环境 |

---

## 整合：完整的解释器

现在你可以把所有部分组合起来：

\`\`\`python
# 完整的解释器流程
source = "(define square (lambda (n) (* n n)))"

# 1. 词法分析：字符串 → Token
tokens = tokenize(source)

# 2. 语法分析：Token → AST
ast = parse(tokens)

# 3. 求值：AST → 结果
env = standard_env()
result = evaluate(ast, env)

# 现在可以使用了
evaluate(parse(tokenize("(square 5)")), env)  # → 25
\`\`\`

---

## 扩展挑战

### 扩展 1：添加新的数据类型 — 字符串

让解释器支持字符串类型和字符串操作。

### 扩展 2：添加新的特殊形式 — let, cond

实现 \`let\` 绑定和 \`cond\` 条件表达式。

### 扩展 3：添加宏

让用户能够定义自己的语法转换规则。

### 扩展 4：添加错误处理

实现 \`try/catch\` 风格的错误处理机制。

### 扩展 5：添加模块系统

让代码可以分文件组织和导入。

---

## 下一步

### 继续学习

推荐资源：
- **SICP**（计算机程序的构造和解释）— 经典中的经典
- **Crafting Interpreters** — 从零实现两个完整的解释器
- **PLAI**（Programming Languages: Application and Interpretation）— 现代 PL 入门

### 相关项目

在 first-principles-cs 中，你可以继续探索：
- **simple-compiler** — 从解释器到编译器，理解编译的本质
- **mini-vm** — 实现一个虚拟机，理解代码如何在机器上运行

---

## 总结

你已经完成了一个完整的解释器！

这个项目教会了你：
1. **程序是数据** — 代码可以被解析、操作、变换
2. **递归的力量** — 用递归优雅地处理嵌套结构
3. **环境是关键** — 变量的作用域和生命周期由环境链决定
4. **闭包的魔力** — 函数可以"记住"它被创建时的环境

恭喜你完成了这个学习之旅！`,
    en: `# Module 6: Integration & Extension

> "How do we extend a language?"

## Congratulations!

If you completed the previous modules, you have built a complete interpreter!

Let's review what you learned:

| Module | What You Learned |
|--------|-----------------|
| Lexer | How to turn strings into Tokens |
| Parser | How to turn Tokens into an AST |
| Environment | How to store and look up variables |
| Basic Eval | How to evaluate expressions |
| Lambda & Closures | How functions "remember" their environment |

---

## Integration: The Complete Interpreter

Now you can put all the pieces together:

\`\`\`python
# The complete interpreter pipeline
source = "(define square (lambda (n) (* n n)))"

# 1. Lexical analysis: String → Tokens
tokens = tokenize(source)

# 2. Parsing: Tokens → AST
ast = parse(tokens)

# 3. Evaluation: AST → Result
env = standard_env()
result = evaluate(ast, env)

# Now we can use it
evaluate(parse(tokenize("(square 5)")), env)  # → 25
\`\`\`

---

## Extension Challenges

### Extension 1: Add a New Data Type — Strings

Add string type support and string operations to the interpreter.

### Extension 2: Add New Special Forms — let, cond

Implement \`let\` bindings and \`cond\` conditional expressions.

### Extension 3: Add Macros

Allow users to define their own syntax transformation rules.

### Extension 4: Add Error Handling

Implement a \`try/catch\` style error handling mechanism.

### Extension 5: Add a Module System

Allow code to be organized across files with imports.

---

## Next Steps

### Continue Learning

Recommended resources:
- **SICP** (Structure and Interpretation of Computer Programs) — The classic of classics
- **Crafting Interpreters** — Build two complete interpreters from scratch
- **PLAI** (Programming Languages: Application and Interpretation) — A modern PL introduction

### Related Projects

In first-principles-cs, you can continue exploring:
- **simple-compiler** — From interpreter to compiler, understanding the essence of compilation
- **mini-vm** — Build a virtual machine, understanding how code runs on a machine

---

## Summary

You have completed a full interpreter!

This project taught you:
1. **Programs are data** — Code can be parsed, manipulated, and transformed
2. **The power of recursion** — Elegantly handling nested structures with recursion
3. **Environment is key** — Variable scope and lifetime are determined by the environment chain
4. **The magic of closures** — Functions can "remember" the environment in which they were created

Congratulations on completing this learning journey!`,
  },
};
