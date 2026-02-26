import type { LearningModule } from "../types";

export const module00Introduction: LearningModule = {
  id: "00-introduction",
  index: 0,
  slug: "introduction",
  hasCode: false,
  vizType: "none",
  readme: {
    zh: `# 模块 0：为什么要学解释器？

> "理解解释器，就是理解程序如何被'读懂'和'执行'的本质。"

## 一个简单的问题

假设你有一个计算器，可以计算 \`(+ 1 2)\` 得到 \`3\`。

现在，用户想要定义变量：

\`\`\`lisp
(define x 10)
(+ x 5)  ; 应该得到 15
\`\`\`

**思考**：你会如何实现这个功能？变量 \`x\` 的值存在哪里？

---

## 更进一步

用户还想要定义函数：

\`\`\`lisp
(define square (lambda (n) (* n n)))
(square 5)  ; 应该得到 25
\`\`\`

**思考**：
- 函数的参数 \`n\` 和外部的变量 \`x\` 会冲突吗？
- 如果有两个函数都用 \`n\` 作为参数呢？

---

## 闭包之谜

这是最有趣的部分。考虑这段代码：

\`\`\`lisp
(define make-adder
  (lambda (x)
    (lambda (y) (+ x y))))

(define add5 (make-adder 5))
(add5 3)  ; 结果是什么？
\`\`\`

**思考**：
- \`make-adder\` 返回了一个函数
- 这个返回的函数里面用到了 \`x\`
- 但是 \`make-adder\` 已经执行完了，\`x\` 还存在吗？
- \`add5\` 是如何"记住" \`x = 5\` 的？

这就是**闭包**的魔力，也是我们这个项目要揭开的核心谜题。

---

## 你将学到什么

完成这个项目后，你将能够回答：

| 问题 | 对应模块 |
|------|---------|
| 程序是如何被"读懂"的？ | 模块 1-2：词法分析 + 语法分析 |
| 变量是如何被"记住"的？ | 模块 3：环境模型 |
| 表达式是如何被"计算"的？ | 模块 4：基础求值 |
| 闭包是如何"捕获"环境的？ | 模块 5：Lambda 与闭包 |

---

## 学习路径

\`\`\`
你在这里
    ↓
模块 0: 引言（激发好奇心）
    ↓
模块 1: 词法分析 ─── "如何把字符串变成有意义的单元？"
    ↓
模块 2: 语法分析 ─── "如何把 token 组织成树状结构？"
    ↓
模块 3: 环境模型 ─── "变量的值存在哪里？"
    ↓
模块 4: 基础求值 ─── "如何计算表达式的值？"
    ↓
模块 5: Lambda 与闭包 ─── "函数如何记住它的环境？"（顿悟时刻！）
    ↓
模块 6: 整合与扩展 ─── "如何扩展语言？"
\`\`\`

---

## 准备好了吗？

点击"下一步"进入模块 1：词法分析。`,
    en: `# Module 0: Why Learn Interpreters?

> "Understanding interpreters means understanding the essence of how programs are 'read' and 'executed'."

## A Simple Question

Suppose you have a calculator that computes \`(+ 1 2)\` to get \`3\`.

Now, the user wants to define variables:

\`\`\`lisp
(define x 10)
(+ x 5)  ; should return 15
\`\`\`

**Think**: How would you implement this? Where is the value of \`x\` stored?

---

## Going Further

The user also wants to define functions:

\`\`\`lisp
(define square (lambda (n) (* n n)))
(square 5)  ; should return 25
\`\`\`

**Think**:
- Will the parameter \`n\` conflict with the external variable \`x\`?
- What if two functions both use \`n\` as a parameter?

---

## The Closure Mystery

This is the most interesting part. Consider this code:

\`\`\`lisp
(define make-adder
  (lambda (x)
    (lambda (y) (+ x y))))

(define add5 (make-adder 5))
(add5 3)  ; What's the result?
\`\`\`

**Think**:
- \`make-adder\` returns a function
- This returned function uses \`x\`
- But \`make-adder\` has already finished executing — does \`x\` still exist?
- How does \`add5\` "remember" that \`x = 5\`?

This is the magic of **closures**, and the core mystery we'll unravel in this project.

---

## What You'll Learn

After completing this project, you'll be able to answer:

| Question | Module |
|----------|--------|
| How is a program "read"? | Modules 1-2: Lexer + Parser |
| How are variables "remembered"? | Module 3: Environment Model |
| How are expressions "evaluated"? | Module 4: Basic Evaluation |
| How do closures "capture" their environment? | Module 5: Lambda & Closures |

---

## Learning Path

\`\`\`
You are here
    ↓
Module 0: Introduction (spark curiosity)
    ↓
Module 1: Lexer ─── "How to turn strings into meaningful units?"
    ↓
Module 2: Parser ─── "How to organize tokens into a tree structure?"
    ↓
Module 3: Environment ─── "Where are variable values stored?"
    ↓
Module 4: Basic Eval ─── "How to compute the value of expressions?"
    ↓
Module 5: Lambda & Closures ─── "How do functions remember their environment?" (Aha moment!)
    ↓
Module 6: Integration ─── "How to extend the language?"
\`\`\`

---

## Ready?

Click "Next" to enter Module 1: Lexer.`,
  },
};
