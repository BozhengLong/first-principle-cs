# 提示 1：define 和基本的 get

## define() 实现

`define` 很简单——直接在当前环境的 bindings 中设置：

```python
def define(self, name: str, value: Any):
    self.bindings[name] = value
```

就这么简单！`define` 总是在当前环境创建绑定。

## 基本的 get() 实现（不考虑父环境）

先实现一个只在当前环境查找的版本：

```python
def get(self, name: str) -> Any:
    if name in self.bindings:
        return self.bindings[name]
    raise NameError(f"Undefined variable: {name}")
```

## 下一步

这个版本的 `get` 还不能从父环境查找。

查看 [hint2.md](hint2.md) 了解如何实现向上查找。
