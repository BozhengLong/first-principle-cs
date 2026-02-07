# 提示 2：处理数字和符号

## read_number()

读取数字的关键点：
1. 记录起始位置（用于错误报告）
2. 处理可能的负号
3. 收集所有数字字符
4. 转换为整数

```python
def read_number(self) -> Token:
    start_line = self.line
    start_column = self.column
    num_str = ''

    # 处理负号
    if self.current_char() == '-':
        num_str += self.advance()

    # 收集数字
    while self.current_char() and self.current_char().isdigit():
        num_str += self.advance()

    return Token(TokenType.NUMBER, int(num_str), start_line, start_column)
```

## read_symbol()

读取符号类似，但使用 `is_symbol_char()` 判断：

```python
def read_symbol(self) -> Token:
    start_line = self.line
    start_column = self.column
    symbol = ''

    while self.current_char() and self.is_symbol_char(self.current_char()):
        symbol += self.advance()

    return Token(TokenType.SYMBOL, symbol, start_line, start_column)
```

## read_boolean()

布尔值以 `#` 开头：

```python
def read_boolean(self) -> Token:
    start_line = self.line
    start_column = self.column

    self.advance()  # 跳过 #
    char = self.current_char()

    if char == 't':
        self.advance()
        return Token(TokenType.BOOLEAN, True, start_line, start_column)
    elif char == 'f':
        self.advance()
        return Token(TokenType.BOOLEAN, False, start_line, start_column)
    else:
        raise LexerError(f"Invalid boolean: #{char}", start_line, start_column)
```

## 下一步

现在你应该能实现 `next_token()` 了。

如果还需要帮助，查看 [hint3.md](hint3.md)。
