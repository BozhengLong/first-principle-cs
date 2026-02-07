# 提示 1：自求值和变量查找

## eval() 的基本结构

```python
def eval(self, node: ASTNode, env: Environment) -> Any:
    # 1. 数字 - 自求值
    if isinstance(node, Number):
        return node.value

    # 2. 布尔值 - 自求值
    if isinstance(node, Boolean):
        return node.value

    # 3. 符号 - 变量查找
    if isinstance(node, Symbol):
        return env.get(node.name)

    # 4. S-表达式 - 特殊形式或函数调用
    if isinstance(node, SExpression):
        # ... 下一个提示
        pass

    raise EvaluatorError(f"Unknown node type: {type(node)}")
```

## 关键点

- Number 和 Boolean 直接返回 `node.value`
- Symbol 需要在环境中查找

## 下一步

查看 [hint2.md](hint2.md) 了解如何处理 S-表达式。
