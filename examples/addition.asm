; addition.asm
; Compute A = 10 + 20 and store the result at address 50.
        LOAD A, 10
        LOAD B, 20
        ADD A, B
        STORE A, 50
        HALT
; Expected final state: A=30, B=20, Memory[50]=30, CPU HALTED.
