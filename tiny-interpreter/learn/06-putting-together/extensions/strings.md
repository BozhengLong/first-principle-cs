# 扩展：添加字符串支持

本指南将帮助你为 Tiny Interpreter 添加字符串支持。

## 目标

支持以下功能：

```lisp
"hello world"                    ; 字符串字面量
(string-append "hello" " world") ; 字符串拼接
(string-length "hello")          ; 字符串长度
(string-ref "hello" 0)           ; 获取字符
(substring "hello" 1 3)          ; 子字符串
```

---

## 步骤 1：修改 Lexer

### 添加 Token 类型

```python
class TokenType(Enum):
    # ... 现有类型
    STRING = auto()  # "hello"
```

### 添加 read_string 方法

```python
def read_string(self) -> Token:
    start_line = self.line
    start_column = self.column

    self.advance()  # 跳过开始的 "
    string = ''

    while self.current_char() and self.current_char() != '"':
        char = self.current_char()

        # 处理转义字符
        if char == '\\':
            self.advance()
            escape_char = self.current_char()
            if escape_char == 'n':
                string += '\n'
            elif escape_char == 't':
                string += '\t'
            elif escape_char == '"':
                string += '"'
            elif escape_char == '\\':
                string += '\\'
            else:
                raise LexerError(f"Unknown escape: \\{escape_char}", ...)
            self.advance()
        else:
            string += self.advance()

    if self.current_char() != '"':
        raise LexerError("Unterminated string", start_line, start_column)

    self.advance()  # 跳过结束的 "
    return Token(TokenType.STRING, string, start_line, start_column)
```

### 修改 next_token

```python
def next_token(self) -> Token:
    # ... 现有代码

    # 字符串
    if char == '"':
        return self.read_string()

    # ... 现有代码
```

---

## 步骤 2：修改 Parser

### 添加 AST 节点

```python
@dataclass
class String:
    value: str
    line: int
    column: int

    def __repr__(self):
        return f"String({self.value!r})"
```

### 修改 parse_atom

```python
def parse_atom(self) -> ASTNode:
    token = self.current_token()

    # ... 现有代码

    if token.type == TokenType.STRING:
        self.advance()
        return String(token.value, token.line, token.column)

    # ... 现有代码
```

---

## 步骤 3：修改 Evaluator

### 处理 String 节点

```python
def eval(self, node, env):
    # ... 现有代码

    if isinstance(node, String):
        return node.value

    # ... 现有代码
```

### 添加字符串内置函数

```python
def create_global_environment(self):
    env = Environment()

    # ... 现有函数

    # 字符串操作
    env.define('string-append', lambda *args: ''.join(args))
    env.define('string-length', lambda s: len(s))
    env.define('string-ref', lambda s, i: s[i])
    env.define('substring', lambda s, start, end: s[start:end])
    env.define('string?', lambda x: isinstance(x, str))
    env.define('string=?', lambda a, b: a == b)

    return env
```

---

## 步骤 4：测试

```python
def test_string_literal():
    evaluator = Evaluator()
    assert evaluator.run('"hello"') == "hello"

def test_string_append():
    evaluator = Evaluator()
    assert evaluator.run('(string-append "hello" " " "world")') == "hello world"

def test_string_length():
    evaluator = Evaluator()
    assert evaluator.run('(string-length "hello")') == 5

def test_escape_sequences():
    evaluator = Evaluator()
    assert evaluator.run('"hello\\nworld"') == "hello\nworld"
```

---

## 进阶挑战

1. **Unicode 支持**：确保正确处理 Unicode 字符
2. **字符串插值**：支持 `"Hello, ${name}!"` 语法
3. **正则表达式**：添加正则表达式支持
