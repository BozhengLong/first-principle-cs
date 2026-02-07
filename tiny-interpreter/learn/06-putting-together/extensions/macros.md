# 扩展：添加宏系统

本指南将帮助你为 Tiny Interpreter 添加宏系统。

## 什么是宏？

宏是在**求值前**转换代码的机制。

```lisp
; 定义宏
(define-macro (when condition body)
  (list 'if condition body #f))

; 使用宏
(when (> x 0)
  (print "positive"))

; 展开后
(if (> x 0) (print "positive") #f)
```

宏的关键：**操作的是代码，不是值**。

---

## 宏 vs 函数

| | 函数 | 宏 |
|---|---|---|
| 参数 | 先求值 | 不求值（是代码） |
| 返回 | 值 | 代码（会被求值） |
| 时机 | 运行时 | 编译时/展开时 |

---

## 实现步骤

### 步骤 1：存储宏定义

```python
class Evaluator:
    def __init__(self):
        self.global_env = self.create_global_environment()
        self.macros = {}  # 宏定义表
```

### 步骤 2：实现 define-macro

```python
def eval_define_macro(self, args: List[ASTNode], env: Environment) -> None:
    """定义宏。

    形式：(define-macro (name params...) body...)
    """
    if len(args) < 2:
        raise EvaluatorError("define-macro expects at least 2 arguments")

    signature = args[0]
    if not isinstance(signature, SExpression) or len(signature.elements) < 1:
        raise EvaluatorError("Invalid macro signature")

    name_node = signature.elements[0]
    if not isinstance(name_node, Symbol):
        raise EvaluatorError("Macro name must be a symbol")

    name = name_node.name
    params = [p.name for p in signature.elements[1:]]
    body = args[1:]

    # 存储宏定义
    self.macros[name] = {
        'params': params,
        'body': body
    }

    return None
```

### 步骤 3：实现宏展开

```python
def expand_macros(self, node: ASTNode) -> ASTNode:
    """递归展开所有宏。"""
    if not isinstance(node, SExpression):
        return node

    if len(node.elements) == 0:
        return node

    first = node.elements[0]

    # 检查是否是宏调用
    if isinstance(first, Symbol) and first.name in self.macros:
        macro = self.macros[first.name]
        args = node.elements[1:]

        if len(args) != len(macro['params']):
            raise EvaluatorError(
                f"Macro {first.name} expects {len(macro['params'])} arguments"
            )

        # 创建宏展开环境
        macro_env = Environment()
        for param, arg in zip(macro['params'], args):
            # 注意：arg 是 AST 节点，不是值！
            macro_env.define(param, arg)

        # 执行宏体，得到新的 AST
        result = None
        for expr in macro['body']:
            result = self.eval_macro_body(expr, macro_env)

        # 递归展开结果
        return self.expand_macros(result)

    # 递归展开子表达式
    return SExpression(
        [self.expand_macros(elem) for elem in node.elements],
        node.line, node.column
    )

def eval_macro_body(self, node: ASTNode, env: Environment) -> ASTNode:
    """在宏环境中求值，返回 AST。"""
    if isinstance(node, Symbol):
        # 如果是宏参数，返回对应的 AST
        if node.name in env.bindings:
            return env.get(node.name)
        return node

    if isinstance(node, SExpression):
        first = node.elements[0] if node.elements else None

        # list 构造新的 S-表达式
        if isinstance(first, Symbol) and first.name == 'list':
            elements = [self.eval_macro_body(e, env) for e in node.elements[1:]]
            return SExpression(elements, node.line, node.column)

        # 递归处理
        return SExpression(
            [self.eval_macro_body(e, env) for e in node.elements],
            node.line, node.column
        )

    return node
```

### 步骤 4：在求值前展开宏

```python
def run(self, source: str) -> Any:
    ast_nodes = parse(source)

    result = None
    for node in ast_nodes:
        # 先展开宏
        expanded = self.expand_macros(node)
        # 再求值
        result = self.eval(expanded, self.global_env)

    return result
```

---

## 示例宏

### when 宏

```lisp
(define-macro (when condition body)
  (list 'if condition body #f))

(when (> 5 3)
  (+ 1 2))
; 展开为：(if (> 5 3) (+ 1 2) #f)
; 结果：3
```

### unless 宏

```lisp
(define-macro (unless condition body)
  (list 'if condition #f body))

(unless (< 5 3)
  (+ 1 2))
; 展开为：(if (< 5 3) #f (+ 1 2))
; 结果：3
```

### for 宏

```lisp
(define-macro (for var from to body)
  (list 'begin
    (list 'define var from)
    (list 'while (list '<= var to)
      (list 'begin
        body
        (list 'set! var (list '+ var 1))))))
```

---

## 测试

```python
def test_when_macro():
    evaluator = Evaluator()
    evaluator.run("""
        (define-macro (when condition body)
          (list 'if condition body #f))
    """)
    result = evaluator.run("(when #t 42)")
    assert result == 42

def test_when_false():
    evaluator = Evaluator()
    evaluator.run("""
        (define-macro (when condition body)
          (list 'if condition body #f))
    """)
    result = evaluator.run("(when #f 42)")
    assert result is False
```

---

## 进阶挑战

1. **卫生宏**：避免变量捕获问题
2. **quasiquote**：支持 `` ` `` 和 `,` 语法
3. **宏展开调试**：显示宏展开过程
