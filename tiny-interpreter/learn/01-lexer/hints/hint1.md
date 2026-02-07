# 提示 1：基本思路

## Lexer 的核心循环

Lexer 的工作可以概括为一个循环：

```
while 还有字符:
    跳过空白
    跳过注释
    根据当前字符决定读取什么 Token
```

## current_char() 和 advance()

这两个方法是基础：

```python
def current_char(self) -> Optional[str]:
    # 检查是否到达末尾
    if self.pos >= len(self.source):
        return None
    return self.source[self.pos]

def advance(self) -> Optional[str]:
    char = self.current_char()
    if char is not None:
        self.pos += 1
        # 更新行列号
        if char == '\n':
            self.line += 1
            self.column = 1
        else:
            self.column += 1
    return char
```

## skip_whitespace()

```python
def skip_whitespace(self):
    while self.current_char() and self.current_char().isspace():
        self.advance()
```

## 下一步

理解了这些基础方法后，尝试实现 `read_number()` 和 `read_symbol()`。

如果还需要帮助，查看 [hint2.md](hint2.md)。
