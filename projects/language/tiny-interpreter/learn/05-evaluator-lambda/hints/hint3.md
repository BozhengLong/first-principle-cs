# 提示 3：调用闭包

## 闭包调用的实现

```python
if isinstance(func, Closure):
    # a. 检查参数数量
    if len(args) != len(func.params):
        raise EvaluatorError(
            f"Function expects {len(func.params)} arguments, got {len(args)}"
        )

    # b. 创建新环境，parent 是 func.env（不是当前 env！）
    func_env = Environment(func.env)  # 这是关键！

    # c. 绑定参数
    for param, arg in zip(func.params, args):
        func_env.define(param, arg)

    # d. 执行函数体
    result = None
    for expr in func.body:
        result = self.eval(expr, func_env)

    # e. 返回结果
    return result
```

## 关键点

`Environment(func.env)` 而不是 `Environment(env)`！

这就是词法作用域的实现：
- 新环境的 parent 是闭包定义时的环境
- 不是调用时的环境

## 完整的 eval_application

```python
def eval_application(self, elements, env):
    func = self.eval(elements[0], env)
    args = [self.eval(arg, env) for arg in elements[1:]]

    # 内置函数
    if callable(func) and not isinstance(func, Closure):
        return func(*args)

    # 闭包
    if isinstance(func, Closure):
        if len(args) != len(func.params):
            raise EvaluatorError(...)

        func_env = Environment(func.env)
        for param, arg in zip(func.params, args):
            func_env.define(param, arg)

        result = None
        for expr in func.body:
            result = self.eval(expr, func_env)
        return result

    raise EvaluatorError(f"Not a function: {func}")
```

## 恭喜！

如果你理解了这些，你就理解了闭包！

这是编程语言中最优雅的概念之一。
