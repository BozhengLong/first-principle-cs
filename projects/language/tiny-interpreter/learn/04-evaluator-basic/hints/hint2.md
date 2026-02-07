# 提示 2：特殊形式

## 处理 S-表达式

```python
if isinstance(node, SExpression):
    # 空列表
    if len(node.elements) == 0:
        return []

    first = node.elements[0]

    # 检查特殊形式
    if isinstance(first, Symbol):
        if first.name == 'define':
            return self.eval_define(node.elements[1:], env)

        if first.name == 'if':
            return self.eval_if(node.elements[1:], env)

        if first.name == 'quote':
            return self.eval_quote(node.elements[1:])

        if first.name == 'begin':
            return self.eval_begin(node.elements[1:], env)

    # 不是特殊形式，是函数调用
    return self.eval_application(node.elements, env)
```

## eval_define()

```python
def eval_define(self, args: List[ASTNode], env: Environment) -> None:
    if len(args) != 2:
        raise EvaluatorError(f"define expects 2 arguments, got {len(args)}")

    name_node = args[0]
    if not isinstance(name_node, Symbol):
        raise EvaluatorError("define expects a symbol as first argument")

    value = self.eval(args[1], env)
    env.define(name_node.name, value)
    return None
```

## eval_if()

```python
def eval_if(self, args: List[ASTNode], env: Environment) -> Any:
    if len(args) != 3:
        raise EvaluatorError(f"if expects 3 arguments, got {len(args)}")

    condition = self.eval(args[0], env)
    if condition:
        return self.eval(args[1], env)
    else:
        return self.eval(args[2], env)
```

## 下一步

查看 [hint3.md](hint3.md) 了解函数调用的实现。
