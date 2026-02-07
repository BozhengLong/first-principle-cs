# 挑战题：基础求值

## 挑战 1：短路求值

`and` 和 `or` 应该是特殊形式还是普通函数？

```lisp
(and #f (/ 1 0))  ; 应该返回 #f，不应该报错
(or #t (/ 1 0))   ; 应该返回 #t，不应该报错
```

**问题**：
1. 如何实现短路求值？
2. `and` 和 `or` 应该返回什么？布尔值还是最后求值的表达式？
3. 如果没有参数，`(and)` 和 `(or)` 应该返回什么？

---

## 挑战 2：多分支条件

`if` 只有两个分支。如果要支持多分支：

```lisp
(cond
  ((< x 0) "negative")
  ((= x 0) "zero")
  (#t "positive"))
```

**问题**：
1. `cond` 应该如何实现？
2. 如果没有条件为真怎么办？
3. `cond` 可以用 `if` 实现吗？

---

## 挑战 3：求值顺序

考虑：

```lisp
(define x 1)
(+ (begin (define x 2) x)
   x)
```

**问题**：
1. 参数是从左到右求值还是从右到左？
2. 结果是 4 还是 3？
3. 不同语言如何处理这个问题？

---

## 挑战 4：尾调用优化

考虑：

```lisp
(define factorial
  (lambda (n acc)
    (if (= n 0)
        acc
        (factorial (- n 1) (* n acc)))))

(factorial 10000 1)  ; 会栈溢出吗？
```

**问题**：
1. 什么是尾调用？
2. 如何检测尾调用？
3. 如何实现尾调用优化？

---

## 挑战 5：错误处理

当前实现遇到错误就崩溃。如果要支持：

```lisp
(try
  (/ 1 0)
  (catch e
    (print "Error:" e)
    0))
```

**问题**：
1. 如何实现 try/catch？
2. 错误应该包含什么信息？
3. 如何实现 finally？

---

## 挑战 6：惰性求值

有些语言使用惰性求值：

```lisp
(define ones (cons 1 ones))  ; 无限列表！
(car ones)  ; 1
(car (cdr ones))  ; 1
```

**问题**：
1. 什么是惰性求值？
2. 如何实现惰性求值？
3. 惰性求值的优缺点是什么？

---

## 延伸阅读

- [SICP - Evaluation](https://mitpress.mit.edu/sites/default/files/sicp/full-text/book/book-Z-H-26.html)
- [Tail Call Optimization](https://en.wikipedia.org/wiki/Tail_call)
- [Lazy Evaluation](https://en.wikipedia.org/wiki/Lazy_evaluation)
