; fibonacci.asm
; Compute Fibonacci numbers iteratively.
; After 10 iterations b = F(11) = 89, stored at address 100.
; F(0)=0, F(1)=1, F(2)=1, F(3)=2, ... F(10)=55, F(11)=89
        LOAD A, 0        ; a = F(0)
        LOAD B, 1        ; b = F(1)
        LOAD C, 10       ; iteration counter
LOOP:
        MOV D, B         ; temp = b
        ADD B, A         ; b = b + a  (next Fibonacci number)
        MOV A, D         ; a = previous b
        DEC C            ; counter = counter - 1
        JNZ LOOP         ; repeat until counter hits 0
        STORE B, 100     ; store F(11) = 89 at address 100
        HALT
; Expected final state: A=55, B=89, C=0, D=55, Memory[100]=89, CPU HALTED.
