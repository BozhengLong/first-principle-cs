; Factorial example - demonstrates recursion

(define factorial
  (lambda (n)
    (if (= n 0)
        1
        (* n (factorial (- n 1))))))

; Test factorial
(factorial 5)  ; Should return 120
