# 提示 3：完整的 next_token() 实现

## next_token() 的完整逻辑

```python
def next_token(self) -> Token:
    # 1. 跳过空白
    self.skip_whitespace()

    # 2. 跳过注释（可能有多个连续注释）
    while self.current_char() == ';':
        self.skip_comment()
        self.skip_whitespace()

    # 3. 获取当前字符
    char = self.current_char()

    # 4. 检查是否到达末尾
    if char is None:
        return Token(TokenType.EOF, None, self.line, self.column)

    # 5. 括号
    if char == '(':
        token = Token(TokenType.LPAREN, '(', self.line, self.column)
        self.advance()
        return token

    if char == ')':
        token = Token(TokenType.RPAREN, ')', self.line, self.column)
        self.advance()
        return token

    # 6. 布尔值
    if char == '#':
        return self.read_boolean()

    # 7. 数字（包括负数）
    # 关键：负号后面必须紧跟数字才是负数
    if char.isdigit() or (char == '-' and self.peek_char() and self.peek_char().isdigit()):
        return self.read_number()

    # 8. 符号
    if self.is_symbol_char(char):
        return self.read_symbol()

    # 9. 无法识别的字符
    raise LexerError(f"Unexpected character: {char!r}", self.line, self.column)
```

## 关键点

### 负数 vs 减号

```python
# 这是负数：-42
# 这是减号：(- 4 2)
# 区别：负号后面紧跟数字

if char == '-' and self.peek_char() and self.peek_char().isdigit():
    return self.read_number()  # 负数
else:
    return self.read_symbol()  # 减号符号
```

### skip_comment()

```python
def skip_comment(self):
    if self.current_char() == ';':
        while self.current_char() and self.current_char() != '\n':
            self.advance()
```

## 完整实现

如果你还是卡住了，可以参考 `src/tiny_interpreter/lexer.py` 中的完整实现。

但建议先自己尝试，理解每一步的原因！
