# 提示 1：Closure 类

## 理解 Closure

Closure 类已经提供了，关键是理解它的三个属性：

```python
class Closure:
    def __init__(self, params, body, env):
        self.params = params  # ['x', 'y'] - 参数名列表
        self.body = body      # [AST...] - 函数体
        self.env = env        # Environment - 定义时的环境！
```

## 为什么 env 是关键？

考虑：

```lisp
(define x 10)
(define f (lambda (y) (+ x y)))
```

当创建 `f` 的闭包时：
- `params = ['y']`
- `body = [(+ x y)]`
- `env = 当前环境（包含 x=10）`

后来调用 `(f 5)` 时，闭包"记得" `x=10`。

## 下一步

查看 [hint2.md](hint2.md) 了解如何实现 `eval_lambda`。
