# 挑战题：词法分析

完成基本实现后，思考这些更深入的问题。

## 挑战 1：负数 vs 减法

考虑这两个表达式：

```lisp
-42      ; 负数
(- 4 2)  ; 减法
```

**问题**：Lexer 如何区分它们？

**提示**：看看 `-` 后面是什么。

**思考**：
- 当前实现是如何处理的？
- 有没有边界情况会出问题？
- 如果允许 `(- -42 3)` 呢？

---

## 挑战 2：支持浮点数

当前 Lexer 只支持整数。如果要支持浮点数：

```lisp
3.14
-0.5
.5      ; 这个合法吗？
5.      ; 这个呢？
```

**问题**：
1. 需要修改哪些方法？
2. `read_number()` 的逻辑会变得多复杂？
3. 如何处理 `1.2.3` 这样的非法输入？

---

## 挑战 3：支持字符串

如果要支持字符串字面量：

```lisp
"hello"
"hello world"
"hello \"world\""  ; 转义引号
"line1\nline2"     ; 转义换行
```

**问题**：
1. 需要添加什么新的 Token 类型？
2. `read_string()` 方法应该如何实现？
3. 如何处理未闭合的字符串 `"hello`？

---

## 挑战 4：错误恢复

当前 Lexer 遇到错误就停止。但好的 Lexer 应该能：

1. 报告错误位置
2. 尝试恢复，继续分析
3. 收集所有错误，一次性报告

**问题**：
- 如何实现错误恢复？
- "恢复"意味着什么？跳过当前字符？跳到下一个空格？
- 这样做有什么好处和坏处？

---

## 挑战 5：性能优化

当前实现每次调用 `current_char()` 都要检查边界。

**问题**：
1. 有没有办法减少边界检查？
2. 如果源代码很长（比如 1MB），有什么优化策略？
3. 能否使用正则表达式来实现 Lexer？优缺点是什么？

---

## 挑战 6：Unicode 支持

当前实现假设每个字符占一个位置。但 Unicode 字符可能：

```lisp
(define π 3.14159)
(define 你好 "world")
```

**问题**：
1. Python 的字符串索引对 Unicode 友好吗？
2. 列号应该按字符数还是字节数计算？
3. 如何处理组合字符（如 `é` = `e` + `´`）？

---

## 延伸阅读

- [Crafting Interpreters - Scanning](https://craftinginterpreters.com/scanning.html)
- [Dragon Book - Lexical Analysis](https://en.wikipedia.org/wiki/Compilers:_Principles,_Techniques,_and_Tools)
- [正则表达式与有限自动机](https://en.wikipedia.org/wiki/Finite-state_machine)
