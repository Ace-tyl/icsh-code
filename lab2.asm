;
; Initialization
;
        .ORIG x3000         ; Start the program at x3000.
        AND R0, R0, #0      ; Clear R0 which will be used to restore the value of f(N)
        ADD R1, R0, #2      ; Set R1 to 2 which will be used to restore the direction (2 or -2)
        ADD R0, R0, #3      ; Set R0 to 3
        LD R2, NADDR        ; Load the data stored in x3102 (N) to R2
        LD R6, MASK         ; Load the modulo mask
        ADD R2, R2, #-1     ; Subtract 1 from R2 because the initial N is 1
        BRz PROGEND         ; To process N == 1 where the program will immediately end
;
; Loop N times to calculate f(N)
;
LOOP    ADD R0, R0, R0      ; Multiply R0 by 2
        ADD R0, R0, R1      ; Add or Subtract R0 by 2
        AND R0, R0, R6      ; R0 modulo 4096 (R0 &= xFFF)
        AND R3, R0, #7      ; Calculate R0 & 7 to judge if it is divisible by 8
        BRz EXIST8          ; If it is divisible by 8
        ADD R3, R0, #0      ; Copy R0 to R3
;
; Judge if the number contains 8
;
        ADD R5, R3, #0      ; Copy the value from R3 to R5
        LD R7, NTHOUS       ; Load -1000 to R7
DIVLOO1 ADD R5, R5, R7      ; Subtract 1000 from R5
        BRzp DIVLOO1        ; If R5 is non-negative then go to the next loop
        LD R7, THOUS        ; Load 1000 to R7
        ADD R5, R5, R7      ; Add 1000 to R5

        AND R4, R4, #0      ; Reset R4
        LD R7, NHUNDR       ; Load -100 to R7
DIVLOO2 ADD R4, R4, #1      ; Add 1 to R4
        ADD R5, R5, R7      ; Subtract 100 from R5
        BRzp DIVLOO2        ; If R5 is non-negative then go to the next loop
        LD R7, HUNDR        ; Load 100 to R7
        ADD R5, R5, R7      ; Add 100 to R5
        ADD R4, R4, #-9     ; Subtract 9 from R4 to judge if R5 / 100 is 8
        BRz EXIST8          ; If it contains 8

        AND R4, R4, #0      ; Reset R4
DIVLOO3 ADD R4, R4, #1      ; Add 1 to R4
        ADD R5, R5, #-10    ; Subtract 10 from R5
        BRzp DIVLOO3        ; If R5 is non-negative then go to the next loop
        ADD R5, R5, #2      ; Add 2 to R5 to judge if R5 % 10 is 8
        BRz EXIST8          ; If it contains 8
        ADD R4, R4, #-9     ; Subtract 9 from R4 to judge if R5 / 10 is 8
        BRz EXIST8          ; If it contains 8
        BRnzp LOOPEND

EXIST8  NOT R1, R1          ; Calculate R1 <= -R1
        ADD R1, R1, #1
LOOPEND ADD R2, R2, #-1     ; Subtract R2 by 1
        BRp LOOP            ; Calculate the next f value

PROGEND ST R0, RADDR        ; Store the value of R0
        TRAP x25            ; Halt

MASK    .FILL x0FFF         ; Make a number 4095 as the modulo mask
THOUS   .FILL #1000         ; Make number 1000
NTHOUS  .FILL #-1000        ; Make number -1000
HUNDR   .FILL #100          ; Make number 100
NHUNDR  .FILL #-100         ; Make number -100
        .END
;
; The address of N and result
;
        .ORIG x3102
NADDR   .BLKW 1             ; Address of N
RADDR   .BLKW 1             ; Address of the result
        .END
