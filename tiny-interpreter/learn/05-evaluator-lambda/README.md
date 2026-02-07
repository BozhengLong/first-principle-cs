# 模块 5：Lambda 与闭包

> "函数如何记住它的环境？"

## 这是整个项目的"顿悟时刻"！

如果你理解了这个模块，你就理解了编程语言中最优雅的概念之一。

---

## 问题引入

考虑这段代码：

```lisp
(define make-adder
  (lambda (x)
    (lambda (y) (+ x y))))

(define add5 (make-adder 5))
(add5 3)  ; 结果是什么？
```

让我们一步步分析：

1. `make-adder` 是一个函数，接受 `x`，返回另一个函数
2. `(make-adder 5)` 调用时，`x = 5`
3. 返回的函数是 `(lambda (y) (+ x y))`
4. 但是！`make-adder` 已经返回了，`x` 还存在吗？

**答案**：存在！因为返回的函数是一个**闭包**。

---

## 核心概念

### 什么是闭包？

闭包 = 函数代码 + 定义时的环境

```
┌─────────────────────────────────┐
│ Closure                         │
│ ┌─────────────────────────────┐ │
│ │ params: [y]                 │ │
│ │ body: (+ x y)               │ │
│ │ env: ──────────────────┐    │ │
│ └─────────────────────────│───┘ │
└───────────────────────────│─────┘
                            │
                            ↓
                    ┌───────────────┐
                    │ Environment   │
                    │ x → 5         │
                    └───────────────┘
```

当闭包被调用时：
1. 创建新环境，**parent 指向闭包捕获的环境**
2. 在新环境中绑定参数
3. 在新环境中执行函数体

### 关键洞察

```python
class Closure:
    def __init__(self, params, body, env):
        self.params = params  # 参数名列表
        self.body = body      # 函数体（AST 节点列表）
        self.env = env        # 定义时的环境 ← 这是关键！
```

**闭包捕获的是定义时的环境，不是调用时的环境！**

---

## 执行过程可视化

```lisp
(define make-adder
  (lambda (x)
    (lambda (y) (+ x y))))

(define add5 (make-adder 5))
(add5 3)
```

### 步骤 1：定义 make-adder

```
全局环境:
┌─────────────────────────────────┐
│ make-adder → Closure            │
│              params: [x]        │
│              body: (lambda ...) │
│              env: 全局环境       │
└─────────────────────────────────┘
```

### 步骤 2：调用 (make-adder 5)

```
全局环境:
┌─────────────────────────────────┐
│ make-adder → ...                │
└─────────────────────────────────┘
         ↑
         │ parent
┌─────────────────────────────────┐
│ x → 5                           │  ← make-adder 的执行环境
└─────────────────────────────────┘

执行 (lambda (y) (+ x y))，创建新闭包：
  Closure {
    params: [y]
    body: (+ x y)
    env: 上面这个环境（x=5 的环境）  ← 捕获！
  }
```

### 步骤 3：add5 = 返回的闭包

```
全局环境:
┌─────────────────────────────────┐
│ make-adder → ...                │
│ add5 → Closure                  │
│        params: [y]              │
│        body: (+ x y)            │
│        env: ─────────────┐      │
└──────────────────────────│──────┘
                           │
                           ↓
                   ┌───────────────┐
                   │ x → 5         │  ← 这个环境被闭包"记住"了
                   └───────────────┘
```

### 步骤 4：调用 (add5 3)

```
add5 的 env:
┌───────────────┐
│ x → 5         │
└───────────────┘
       ↑
       │ parent
┌───────────────┐
│ y → 3         │  ← 新创建的执行环境
└───────────────┘

求值 (+ x y):
  x → 在当前环境没有，去 parent 找 → 5
  y → 在当前环境找到 → 3
  结果：8
```

---

## 动手实现

### 步骤 1：定义 Closure 类

```python
class Closure:
    def __init__(self, params, body, env):
        self.params = params  # List[str]
        self.body = body      # List[ASTNode]
        self.env = env        # Environment
```

### 步骤 2：实现 eval_lambda

当遇到 `(lambda (params) body)` 时，创建闭包：

```python
def eval_lambda(self, args, env):
    # 提取参数名
    # 提取函数体
    # 创建 Closure，捕获当前环境
    pass
```

### 步骤 3：修改 eval_application

当调用的是 Closure 时：

```python
def eval_application(self, elements, env):
    func = self.eval(elements[0], env)
    args = [self.eval(arg, env) for arg in elements[1:]]

    if isinstance(func, Closure):
        # 创建新环境，parent 是闭包的 env
        # 绑定参数
        # 执行函数体
        pass
```

### 步骤 4：运行测试

```bash
cd tiny-interpreter
pytest learn/05-evaluator-lambda/test_skeleton.py -v
```

---

## 提示

如果卡住了，可以查看提示：

- [提示 1：Closure 类](hints/hint1.md)
- [提示 2：eval_lambda](hints/hint2.md)
- [提示 3：调用闭包](hints/hint3.md)

---

## 深入思考

完成实现后，思考这些问题（详见 [challenge.md](challenge.md)）：

1. 为什么闭包要捕获定义时的环境，而不是调用时的？
2. 闭包会导致内存泄漏吗？
3. 如何实现递归函数？

---

## 恭喜！

如果你理解了闭包，你已经掌握了编程语言中最核心的概念之一。

这个概念在 JavaScript、Python、Ruby、Swift 等现代语言中无处不在。

[进入模块 6：整合与扩展 →](../06-putting-together/README.md)
