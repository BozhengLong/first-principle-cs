# 挑战题：环境模型

## 挑战 1：动态作用域 vs 词法作用域

我们实现的是**词法作用域**。但有些语言使用**动态作用域**。

```lisp
(define x 10)

(define f (lambda () x))

(define g
  (lambda ()
    (define x 20)
    (f)))

(g)  ; 词法作用域：10，动态作用域：20
```

**问题**：
1. 词法作用域和动态作用域的区别是什么？
2. 如何修改 Environment 实现动态作用域？
3. 各有什么优缺点？

---

## 挑战 2：变量遮蔽的边界情况

```lisp
(define x 10)

(define f
  (lambda (x)
    (define x 20)  ; 这合法吗？
    x))

(f 5)  ; 结果是什么？
```

**问题**：
1. 在函数内部 `define` 一个同名参数，应该允许吗？
2. 不同语言如何处理这种情况？
3. 我们的实现会怎么处理？

---

## 挑战 3：闭包与环境

```lisp
(define make-counter
  (lambda ()
    (define count 0)
    (lambda ()
      (set! count (+ count 1))
      count)))

(define c1 (make-counter))
(define c2 (make-counter))

(c1)  ; 1
(c1)  ; 2
(c2)  ; 1 还是 3？
```

**问题**：
1. `c1` 和 `c2` 共享环境吗？
2. 为什么每次调用 `make-counter` 会创建独立的计数器？
3. 画出 `c1` 和 `c2` 的环境链。

---

## 挑战 4：环境的内存管理

考虑这段代码：

```lisp
(define f
  (lambda ()
    (define huge-data (make-huge-list 1000000))
    (lambda () 42)))

(define g (f))
```

**问题**：
1. `huge-data` 什么时候可以被垃圾回收？
2. 闭包会导致内存泄漏吗？
3. 如何优化？（提示：研究"闭包转换"）

---

## 挑战 5：实现 let 和 let*

很多 Lisp 方言有 `let` 语法：

```lisp
; let - 并行绑定
(let ((x 1)
      (y 2))
  (+ x y))

; let* - 顺序绑定
(let* ((x 1)
       (y (+ x 1)))  ; 可以引用前面的 x
  (+ x y))
```

**问题**：
1. `let` 和 `let*` 的区别是什么？
2. 如何用 `lambda` 实现 `let`？
3. 如何用 `lambda` 实现 `let*`？

---

## 挑战 6：实现模块系统

如果要支持模块：

```lisp
; math.lisp
(module math
  (export square cube)
  (define square (lambda (x) (* x x)))
  (define cube (lambda (x) (* x x x)))
  (define helper (lambda (x) x)))  ; 不导出

; main.lisp
(import math)
(math.square 5)  ; 25
(math.helper 5)  ; 错误：helper 未导出
```

**问题**：
1. 模块的环境应该如何组织？
2. 如何实现 `export` 和 `import`？
3. 如何处理循环导入？

---

## 延伸阅读

- [SICP - Environment Model](https://mitpress.mit.edu/sites/default/files/sicp/full-text/book/book-Z-H-21.html)
- [Lexical vs Dynamic Scope](https://en.wikipedia.org/wiki/Scope_(computer_science))
- [Closure Conversion](https://en.wikipedia.org/wiki/Lambda_lifting)
