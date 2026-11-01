; subtraction.asm
; Compute A = 50 - 20 and store the result at address 60.
        LOAD A, 50
        LOAD B, 20
        SUB A, B
        STORE A, 60
        HALT
; Expected final state: A=30, B=20, Memory[60]=30, CPU HALTED.
