# 提示 3：函数调用

## eval_application()

函数调用的步骤：
1. 求值函数（第一个元素）
2. 求值所有参数
3. 调用函数

```python
def eval_application(self, elements: List[ASTNode], env: Environment) -> Any:
    # 1. 求值函数
    func = self.eval(elements[0], env)

    # 2. 求值参数
    args = [self.eval(arg, env) for arg in elements[1:]]

    # 3. 调用函数（目前只处理内置函数）
    if callable(func):
        return func(*args)

    raise EvaluatorError(f"Not a function: {func}")
```

## eval_quote()

```python
def eval_quote(self, args: List[ASTNode]) -> Any:
    if len(args) != 1:
        raise EvaluatorError(f"quote expects 1 argument, got {len(args)}")

    return self.ast_to_value(args[0])
```

## eval_begin()

```python
def eval_begin(self, args: List[ASTNode], env: Environment) -> Any:
    result = None
    for expr in args:
        result = self.eval(expr, env)
    return result
```

## 完整实现

如果你还是卡住了，可以参考 `src/tiny_interpreter/evaluator.py` 中的完整实现。

但注意：完整实现包含了 lambda 和闭包，那是模块 5 的内容！
