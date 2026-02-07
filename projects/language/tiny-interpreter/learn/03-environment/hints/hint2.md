# 提示 2：向上查找

## get() 的完整实现

关键是：如果当前环境没有，就去父环境找。

```python
def get(self, name: str) -> Any:
    # 1. 先在当前环境查找
    if name in self.bindings:
        return self.bindings[name]

    # 2. 如果有父环境，递归查找
    if self.parent is not None:
        return self.parent.get(name)

    # 3. 到达顶层还没找到，报错
    raise NameError(f"Undefined variable: {name}")
```

## 递归 vs 循环

上面用的是递归。也可以用循环：

```python
def get(self, name: str) -> Any:
    env = self
    while env is not None:
        if name in env.bindings:
            return env.bindings[name]
        env = env.parent
    raise NameError(f"Undefined variable: {name}")
```

两种方式都可以，递归更简洁，循环更高效（避免栈溢出）。

## 下一步

现在尝试实现 `set()`。

查看 [hint3.md](hint3.md) 了解 `set` 的实现。
