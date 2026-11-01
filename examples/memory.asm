; memory.asm
; Demonstrate memory stores and a memory round-trip via the stack (PUSH/POP).
        LOAD A, 5
        LOAD B, 10
        ADD A, B         ; A = 15
        STORE A, 10      ; Memory[10] = 15

        LOAD C, 3
        SUB A, C         ; A = 12
        STORE A, 11      ; Memory[11] = 12

        PUSH A           ; push A onto the stack
        POP D            ; pop into D  -> D = A = 12 (copy via memory)
        STORE D, 12      ; Memory[12] = 12
        HALT
; Expected final state: A=12, B=10, C=3, D=12,
;   Memory[10]=15, Memory[11]=12, Memory[12]=12, CPU HALTED.
