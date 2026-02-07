# 提示 2：递归解析

## parse_atom()

解析原子很直接——根据 Token 类型创建对应的 AST 节点：

```python
def parse_atom(self) -> ASTNode:
    token = self.current_token()

    if token.type == TokenType.NUMBER:
        self.advance()
        return Number(token.value, token.line, token.column)

    if token.type == TokenType.BOOLEAN:
        self.advance()
        return Boolean(token.value, token.line, token.column)

    if token.type == TokenType.SYMBOL:
        self.advance()
        return Symbol(token.value, token.line, token.column)

    raise ParserError(
        f"Unexpected token: {token.type.name}",
        token.line,
        token.column
    )
```

## parse_expr()

这是入口点——决定调用 `parse_atom()` 还是 `parse_sexp()`：

```python
def parse_expr(self) -> ASTNode:
    token = self.current_token()

    if token.type == TokenType.LPAREN:
        return self.parse_sexp()
    else:
        return self.parse_atom()
```

## 下一步

现在尝试实现 `parse_sexp()`。这是最关键的部分！

如果还需要帮助，查看 [hint3.md](hint3.md)。
