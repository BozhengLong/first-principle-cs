# 提示 3：set 的实现

## set() vs define()

关键区别：
- `define`：总是在**当前**环境创建绑定
- `set`：修改**已存在**的变量，可能在父环境中

## set() 的完整实现

```python
def set(self, name: str, value: Any):
    # 1. 如果当前环境有这个变量，修改它
    if name in self.bindings:
        self.bindings[name] = value
        return

    # 2. 如果有父环境，递归查找并修改
    if self.parent is not None:
        self.parent.set(name, value)
        return

    # 3. 到达顶层还没找到，报错
    raise NameError(f"Undefined variable: {name}")
```

## 为什么 set 不创建新变量？

考虑这个例子：

```lisp
(define x 10)

(define f
  (lambda ()
    (set! x 20)))  ; 应该修改全局的 x

(f)
x  ; 应该是 20
```

如果 `set` 在当前环境创建新变量，就会遮蔽全局的 `x`，而不是修改它。

## 完整实现

如果你还是卡住了，可以参考 `src/tiny_interpreter/environment.py` 中的完整实现。
