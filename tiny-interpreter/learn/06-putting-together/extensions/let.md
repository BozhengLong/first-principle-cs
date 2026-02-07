# 扩展：添加 let 表达式

本指南将帮助你为 Tiny Interpreter 添加 `let` 和 `let*` 表达式。

## 目标

支持以下功能：

```lisp
; let - 并行绑定
(let ((x 1)
      (y 2))
  (+ x y))  ; 3

; let* - 顺序绑定
(let* ((x 1)
       (y (+ x 1)))  ; y 可以引用 x
  (+ x y))  ; 3
```

---

## 理解 let

`let` 可以看作是 `lambda` 的语法糖：

```lisp
(let ((x 1) (y 2))
  (+ x y))

; 等价于

((lambda (x y) (+ x y)) 1 2)
```

---

## 方法 1：在 Evaluator 中直接实现

### 添加 eval_let

```python
def eval_let(self, args: List[ASTNode], env: Environment) -> Any:
    """求值 let 表达式。

    形式：(let ((var1 val1) (var2 val2) ...) body...)
    """
    if len(args) < 2:
        raise EvaluatorError("let expects at least 2 arguments")

    bindings_node = args[0]
    if not isinstance(bindings_node, SExpression):
        raise EvaluatorError("let expects a list of bindings")

    # 创建新环境
    let_env = Environment(env)

    # 并行求值所有绑定值（在原环境中）
    values = []
    names = []
    for binding in bindings_node.elements:
        if not isinstance(binding, SExpression) or len(binding.elements) != 2:
            raise EvaluatorError("Invalid let binding")

        name_node = binding.elements[0]
        if not isinstance(name_node, Symbol):
            raise EvaluatorError("let binding name must be a symbol")

        names.append(name_node.name)
        values.append(self.eval(binding.elements[1], env))  # 在原环境求值！

    # 在新环境中绑定
    for name, value in zip(names, values):
        let_env.define(name, value)

    # 执行 body
    result = None
    for expr in args[1:]:
        result = self.eval(expr, let_env)

    return result
```

### 添加 eval_let_star

```python
def eval_let_star(self, args: List[ASTNode], env: Environment) -> Any:
    """求值 let* 表达式。

    形式：(let* ((var1 val1) (var2 val2) ...) body...)

    与 let 的区别：每个绑定可以引用前面的绑定。
    """
    if len(args) < 2:
        raise EvaluatorError("let* expects at least 2 arguments")

    bindings_node = args[0]
    if not isinstance(bindings_node, SExpression):
        raise EvaluatorError("let* expects a list of bindings")

    # 创建新环境
    let_env = Environment(env)

    # 顺序求值和绑定
    for binding in bindings_node.elements:
        if not isinstance(binding, SExpression) or len(binding.elements) != 2:
            raise EvaluatorError("Invalid let* binding")

        name_node = binding.elements[0]
        if not isinstance(name_node, Symbol):
            raise EvaluatorError("let* binding name must be a symbol")

        # 在当前 let_env 中求值（可以引用前面的绑定）
        value = self.eval(binding.elements[1], let_env)
        let_env.define(name_node.name, value)

    # 执行 body
    result = None
    for expr in args[1:]:
        result = self.eval(expr, let_env)

    return result
```

### 修改 eval 方法

```python
def eval(self, node, env):
    # ... 现有代码

    if isinstance(first, Symbol):
        # ... 现有特殊形式

        if first.name == 'let':
            return self.eval_let(node.elements[1:], env)

        if first.name == 'let*':
            return self.eval_let_star(node.elements[1:], env)

    # ... 现有代码
```

---

## 方法 2：转换为 lambda（宏展开）

另一种方法是在求值前将 `let` 转换为 `lambda`：

```python
def expand_let(self, node: ASTNode) -> ASTNode:
    """将 let 展开为 lambda 调用。"""
    if not isinstance(node, SExpression):
        return node

    if len(node.elements) == 0:
        return node

    first = node.elements[0]
    if isinstance(first, Symbol) and first.name == 'let':
        # (let ((x 1) (y 2)) body)
        # → ((lambda (x y) body) 1 2)

        bindings = node.elements[1]
        body = node.elements[2:]

        params = []
        args = []
        for binding in bindings.elements:
            params.append(binding.elements[0])
            args.append(binding.elements[1])

        lambda_node = SExpression(
            [Symbol('lambda', ...), SExpression(params, ...), *body],
            ...
        )
        return SExpression([lambda_node, *args], ...)

    # 递归展开子表达式
    return SExpression(
        [self.expand_let(elem) for elem in node.elements],
        node.line, node.column
    )
```

---

## 测试

```python
def test_let_basic():
    evaluator = Evaluator()
    result = evaluator.run("(let ((x 1) (y 2)) (+ x y))")
    assert result == 3

def test_let_parallel():
    evaluator = Evaluator()
    # x 和 y 的值在同一环境中求值
    evaluator.run("(define x 10)")
    result = evaluator.run("(let ((x 1) (y x)) y)")
    assert result == 10  # y 引用的是外部的 x=10

def test_let_star_sequential():
    evaluator = Evaluator()
    result = evaluator.run("(let* ((x 1) (y (+ x 1))) (+ x y))")
    assert result == 3  # y 可以引用 x=1

def test_let_nested():
    evaluator = Evaluator()
    result = evaluator.run("""
        (let ((x 1))
          (let ((y 2))
            (+ x y)))
    """)
    assert result == 3
```

---

## 进阶挑战

1. **letrec**：支持递归绑定
2. **named let**：支持 `(let loop ((i 0)) ...)`
3. **解构绑定**：支持 `(let (((x y) (list 1 2))) ...)`
