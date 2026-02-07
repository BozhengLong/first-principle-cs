# 提示 3：完整的 parse_sexp() 实现

## parse_sexp() 的完整逻辑

```python
def parse_sexp(self) -> SExpression:
    # 1. 消费左括号，记录位置
    lparen = self.expect(TokenType.LPAREN)

    # 2. 收集元素
    elements = []
    while self.current_token().type != TokenType.RPAREN:
        # 检查是否意外到达文件末尾
        if self.current_token().type == TokenType.EOF:
            raise ParserError(
                "Unexpected EOF, expected ')'",
                self.current_token().line,
                self.current_token().column
            )
        # 递归解析元素
        elements.append(self.parse_expr())

    # 3. 消费右括号
    self.expect(TokenType.RPAREN)

    # 4. 返回 S-表达式节点
    return SExpression(elements, lparen.line, lparen.column)
```

## 关键点

### 递归的魔力

注意 `parse_sexp()` 调用 `parse_expr()`，而 `parse_expr()` 可能又调用 `parse_sexp()`。

这种相互递归自然地处理了任意深度的嵌套：

```
(+ 1 (* 2 (- 3 4)))
```

### 错误处理

两个重要的错误情况：
1. 期望 `)` 但遇到 EOF → 括号未闭合
2. 期望原子但遇到 `)` → 意外的右括号

## 完整实现

如果你还是卡住了，可以参考 `src/tiny_interpreter/parser.py` 中的完整实现。

但建议先自己尝试，理解递归下降的工作原理！
