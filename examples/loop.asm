; loop.asm
; Count down from 5 to 0 using DEC and a conditional jump (JNZ).
; Demonstrates: branching, labels, and the Zero flag driving a loop.
        LOAD A, 5        ; counter = 5
LOOP:
        DEC A            ; A = A - 1  (sets Z when A reaches 0)
        JNZ LOOP         ; loop while A != 0
        HALT
; Expected final state: A=0, Z=1, CPU HALTED.
