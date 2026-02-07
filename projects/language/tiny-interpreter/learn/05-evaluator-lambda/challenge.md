# 挑战题：Lambda 与闭包

## 挑战 1：词法作用域 vs 动态作用域

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
1. 我们的实现是词法作用域还是动态作用域？
2. 如何修改实现变成动态作用域？
3. 为什么大多数现代语言选择词法作用域？

---

## 挑战 2：递归函数

```lisp
(define factorial
  (lambda (n)
    (if (= n 0)
        1
        (* n (factorial (- n 1))))))
```

**问题**：
1. 在 `factorial` 的函数体中，`factorial` 是如何被找到的？
2. 如果 `define` 先求值再绑定，会有问题吗？
3. 如何实现匿名递归（Y 组合子）？

---

## 挑战 3：闭包与可变状态

```lisp
(define make-counter
  (lambda ()
    (define count 0)
    (lambda ()
      (set! count (+ count 1))
      count)))

(define c (make-counter))
(c)  ; 1
(c)  ; 2
(c)  ; 3
```

**问题**：
1. 为什么每次调用 `c` 都会增加？
2. `count` 存在哪里？
3. 这和面向对象的"对象"有什么关系？

---

## 挑战 4：多参数与柯里化

```lisp
; 普通多参数函数
(define add (lambda (x y) (+ x y)))
(add 1 2)  ; 3

; 柯里化版本
(define add-curried
  (lambda (x)
    (lambda (y) (+ x y))))
((add-curried 1) 2)  ; 3
```

**问题**：
1. 柯里化有什么好处？
2. 如何自动将多参数函数转换为柯里化形式？
3. Haskell 为什么默认使用柯里化？

---

## 挑战 5：闭包的内存管理

```lisp
(define f
  (lambda ()
    (define huge-list (make-list 1000000))
    (define small-value 42)
    (lambda () small-value)))

(define g (f))
```

**问题**：
1. `huge-list` 什么时候可以被垃圾回收？
2. 闭包是否应该只捕获实际使用的变量？
3. 研究一下"闭包转换"（closure conversion）。

---

## 挑战 6：实现 let 和 letrec

```lisp
; let 可以用 lambda 实现
(let ((x 1) (y 2))
  (+ x y))
; 等价于
((lambda (x y) (+ x y)) 1 2)

; letrec 允许递归定义
(letrec ((even? (lambda (n)
                  (if (= n 0) #t (odd? (- n 1)))))
         (odd? (lambda (n)
                 (if (= n 0) #f (even? (- n 1))))))
  (even? 10))
```

**问题**：
1. 如何用 lambda 实现 `let`？
2. `letrec` 为什么需要特殊处理？
3. 如何实现相互递归的函数？

---

## 挑战 7：Y 组合子

不使用 `define`，如何实现递归？

```lisp
; Y 组合子
(define Y
  (lambda (f)
    ((lambda (x) (f (lambda (y) ((x x) y))))
     (lambda (x) (f (lambda (y) ((x x) y)))))))

; 使用 Y 组合子实现阶乘
(define factorial
  (Y (lambda (fact)
       (lambda (n)
         (if (= n 0)
             1
             (* n (fact (- n 1))))))))
```

**问题**：
1. Y 组合子是如何工作的？
2. 为什么需要 `(lambda (y) ((x x) y))` 而不是 `(x x)`？
3. 这和"不动点"有什么关系？

---

## 延伸阅读

- [SICP - Procedures and Processes](https://mitpress.mit.edu/sites/default/files/sicp/full-text/book/book-Z-H-11.html)
- [Lambda Calculus](https://en.wikipedia.org/wiki/Lambda_calculus)
- [Y Combinator](https://en.wikipedia.org/wiki/Fixed-point_combinator)
- [Closure Conversion](https://en.wikipedia.org/wiki/Lambda_lifting)
