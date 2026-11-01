; acceptance.asm
; The program from section 34 of the project spec. The whole project is
; considered functional only when this runs correctly.
START:
    LOAD A, 10
    LOAD B, 20
    ADD A, B
    STORE A, 50

    LOAD C, 5
    SUB A, C
    STORE A, 51

    HALT
; Expected: A=25, B=20, C=5, Memory[50]=30, Memory[51]=25, CPU HALTED.
