# 提示 2：eval_lambda

## eval_lambda 的实现

```python
def eval_lambda(self, args: List[ASTNode], env: Environment) -> Closure:
    # 1. 检查参数数量
    if len(args) < 2:
        raise EvaluatorError("lambda expects at least 2 arguments")

    # 2. 提取参数列表
    params_node = args[0]
    if not isinstance(params_node, SExpression):
        raise EvaluatorError("lambda expects a list of parameters")

    # 3. 验证并提取参数名
    params = []
    for param in params_node.elements:
        if not isinstance(param, Symbol):
            raise EvaluatorError("lambda parameters must be symbols")
        params.append(param.name)

    # 4. 提取函数体
    body = args[1:]

    # 5. 创建闭包，捕获当前环境
    return Closure(params, body, env)  # env 是关键！
```

## 关键点

最后一行 `Closure(params, body, env)` 中的 `env` 是当前环境。

这就是闭包"捕获"环境的地方！

## 下一步

查看 [hint3.md](hint3.md) 了解如何调用闭包。
