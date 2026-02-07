# 提示 1：基本结构

## Parser 的核心方法

Parser 需要几个基础方法来操作 Token 流：

```python
def current_token(self) -> Token:
    """返回当前 Token。"""
    if self.pos >= len(self.tokens):
        return self.tokens[-1]  # 返回 EOF
    return self.tokens[self.pos]

def advance(self) -> Token:
    """消费当前 Token 并前进。"""
    token = self.current_token()
    if self.pos < len(self.tokens) - 1:
        self.pos += 1
    return token
```

## expect() 方法

这个方法用于确保当前 Token 是期望的类型：

```python
def expect(self, token_type: TokenType) -> Token:
    token = self.current_token()
    if token.type != token_type:
        raise ParserError(
            f"Expected {token_type.name}, got {token.type.name}",
            token.line,
            token.column
        )
    return self.advance()
```

## 下一步

理解了这些基础方法后，尝试实现 `parse_atom()`。

如果还需要帮助，查看 [hint2.md](hint2.md)。
