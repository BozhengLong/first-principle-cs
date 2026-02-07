# 模块 6：整合与扩展

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

```python
from lexer import Lexer
from parser import Parser
from evaluator import Evaluator

def run(source: str):
    # 1. 词法分析
    lexer = Lexer(source)
    tokens = lexer.tokenize()

    # 2. 语法分析
    parser = Parser(tokens)
    ast = parser.parse()

    # 3. 求值
    evaluator = Evaluator()
    result = None
    for node in ast:
        result = evaluator.eval(node, evaluator.global_env)

    return result
```

---

## 扩展挑战

现在你已经理解了解释器的核心，可以尝试扩展它！

### 扩展 1：添加新的数据类型

**字符串**

```lisp
"hello world"
(string-append "hello" " " "world")
(string-length "hello")
```

需要修改：
- Lexer：识别字符串字面量
- Parser：创建 String AST 节点
- Evaluator：处理字符串操作

### 扩展 2：添加新的特殊形式

**let 表达式**

```lisp
(let ((x 1) (y 2))
  (+ x y))
```

提示：`let` 可以转换为 `lambda` 调用：

```lisp
((lambda (x y) (+ x y)) 1 2)
```

**cond 表达式**

```lisp
(cond
  ((< x 0) "negative")
  ((= x 0) "zero")
  (#t "positive"))
```

### 扩展 3：添加宏

宏在求值前转换代码：

```lisp
(define-macro (when condition body)
  (list 'if condition body #f))

(when (> x 0)
  (print "positive"))
; 展开为：(if (> x 0) (print "positive") #f)
```

### 扩展 4：添加错误处理

```lisp
(try
  (/ 1 0)
  (catch e
    (print "Error:" e)
    0))
```

### 扩展 5：添加模块系统

```lisp
; math.lisp
(module math
  (export square cube)
  (define square (lambda (x) (* x x)))
  (define cube (lambda (x) (* x x x))))

; main.lisp
(import math)
(math.square 5)
```

---

## 扩展指南

每个扩展都在 `extensions/` 目录下有详细指南：

- [extensions/strings.md](extensions/strings.md) - 添加字符串支持
- [extensions/let.md](extensions/let.md) - 添加 let 表达式
- [extensions/macros.md](extensions/macros.md) - 添加宏系统

---

## 下一步

### 继续学习

- **SICP**：《计算机程序的构造和解释》是学习解释器的经典教材
- **Crafting Interpreters**：一本优秀的现代解释器教程
- **PLAI**：《Programming Languages: Application and Interpretation》

### 相关项目

在 first-principles-cs 课程中，还有其他相关项目：

- **simple-compiler**：从解释器到编译器
- **mini-vm**：实现一个虚拟机

---

## 总结

你已经完成了一个完整的解释器！

这个项目教会了你：

1. **程序是数据**：源代码只是字符串，AST 只是数据结构
2. **递归的力量**：解析和求值都是递归的
3. **环境是关键**：变量查找依赖环境链
4. **闭包的魔力**：函数可以"记住"定义时的环境

这些概念在所有编程语言中都是通用的。

**恭喜你完成了这个学习之旅！**
