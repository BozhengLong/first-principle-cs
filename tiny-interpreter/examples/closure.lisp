; Closure example - demonstrates lexical scoping

(define make-counter
  (lambda (init)
    (lambda ()
      (begin
        (define result init)
        (define init (+ init 1))
        result))))

; Create a counter starting at 0
(define counter (make-counter 0))

; Call the counter multiple times
(counter)  ; Returns 0
(counter)  ; Returns 1
(counter)  ; Returns 2
