# 挑战题：整合与扩展

## 挑战 1：实现 REPL

实现一个交互式的 Read-Eval-Print Loop：

```
> (define x 10)
> (+ x 5)
15
> (define square (lambda (n) (* n n)))
> (square x)
100
> (exit)
Goodbye!
```

**要求**：
1. 支持多行输入（括号未闭合时继续读取）
2. 显示友好的错误信息
3. 支持历史记录（上下箭头）
4. 支持 `(exit)` 退出

---

## 挑战 2：实现调试器

添加调试功能：

```lisp
(debug
  (define x 10)
  (define y (+ x 5))  ; 在这里暂停
  (* x y))
```

**要求**：
1. 单步执行
2. 查看当前环境
3. 查看调用栈
4. 设置断点

---

## 挑战 3：实现类型检查

添加静态类型检查：

```lisp
(define (square : (-> Int Int))
  (lambda (x : Int) (* x x)))

(square "hello")  ; 类型错误！
```

**要求**：
1. 基本类型：Int, Bool, String
2. 函数类型：(-> T1 T2)
3. 类型推断（可选）

---

## 挑战 4：实现垃圾回收

当前实现依赖 Python 的垃圾回收。如果要自己实现：

**要求**：
1. 标记-清除算法
2. 引用计数
3. 分代回收（可选）

---

## 挑战 5：编译到字节码

不直接解释 AST，而是先编译到字节码：

```
源代码: (+ 1 (* 2 3))

字节码:
  PUSH 1
  PUSH 2
  PUSH 3
  MUL
  ADD

执行字节码（栈机器）
```

**要求**：
1. 设计字节码指令集
2. 实现编译器（AST → 字节码）
3. 实现虚拟机（执行字节码）

---

## 挑战 6：实现并发

添加并发支持：

```lisp
(define counter 0)

(spawn
  (lambda ()
    (set! counter (+ counter 1))))

(spawn
  (lambda ()
    (set! counter (+ counter 1))))

(sleep 100)
counter  ; 可能是 1 或 2，取决于执行顺序
```

**要求**：
1. 实现 `spawn` 创建线程
2. 实现 `channel` 进行通信
3. 处理竞态条件

---

## 挑战 7：实现面向对象

添加对象系统：

```lisp
(define Point
  (class ()
    (field x 0)
    (field y 0)
    (method (init self x y)
      (set! (self x) x)
      (set! (self y) y))
    (method (distance self other)
      (sqrt (+ (square (- (self x) (other x)))
               (square (- (self y) (other y))))))))

(define p1 (new Point 0 0))
(define p2 (new Point 3 4))
(p1 distance p2)  ; 5
```

**要求**：
1. 类定义
2. 实例化
3. 方法调用
4. 继承（可选）

---

## 延伸阅读

- [Crafting Interpreters](https://craftinginterpreters.com/)
- [SICP](https://mitpress.mit.edu/sites/default/files/sicp/full-text/book/book.html)
- [PLAI](https://www.plai.org/)
- [Essentials of Programming Languages](https://eopl3.com/)
