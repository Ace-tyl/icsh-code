        .ORIG x3000
;
; Read the number N, initialize and call the function
; N is stored to R1, stack top is R2 and output pointer is R3
; The state is stored in R4
;
        LDI R1, NADRES
        LD R3, OUTPTR
        LD R2, STKPTR
        AND R4, R4, #0      ; Clear register R4
        JSR REMOVE          ; Execute REMOVE(N)
        HALT
;
; Implement the OPRING function
; The operation is in R5
;
OPRING  ADD R4, R4, R5      ; Make the operation
        STR R4, R3, #0      ; Save the state to output pointer (R3)
        ADD R3, R3, #1      ; Add the output pointer by 1
        RET                 ; Return
;
; Implement the SHLEFT function
; The N is in R1 and will save 1 << (N - 1) in R5
;
SHLEFT  ADD R6, R1, #-1     ; Set R6 to R1 - 1
        AND R5, R5, #0      ; Clear R5
        ADD R5, R5, #1      ; Set R5 to 1
SLOOP   BRz SRETN           ; If R6 is 0 return
        ADD R5, R5, R5      ; Multiply R5 by 2
        ADD R6, R6, #-1     ; Subtract R6 by 1
        BRnzp SLOOP         ; Next loop
SRETN   RET                 ; Return
;
; Implement the REMOVE function
; The N is in R1 and will be saved into stack
;
REMOVE  STR R7, R2, #0      ; Save R7 to stack top
        STR R1, R2, #1      ; Save R1 to stack top
        ADD R2, R2, #2      ; Add stack top by 2
        ADD R1, R1, #-1     ; Subtract R1 by 1
        BRn RRETN           ; If N is 0 (now R1 is -1), return
        ADD R1, R1, #-1     ; Subtract R1 by 1
        BRn RNIS1           ; If N is 1 (now R1 is -1), jump to special process
        JSR REMOVE          ; Remove N - 2
        LDR R1, R2, #-1     ; Load R1 from stack
        JSR SHLEFT          ; Get 1 << (N - 1) to R5
        JSR OPRING          ; Operate
        ADD R1, R1, #-2     ; Subtract R1 by 2
        JSR PUT             ; Put N - 2
        LDR R1, R2, #-1     ; Load R1 from stack
        ADD R1, R1, #-1     ; Subtract R1 by 1
        JSR REMOVE          ; Remove N - 1
RRETN   LDR R7, R2, #-2     ; Load the previous address from stack
        ADD R2, R2, #-2     ; Subtract stack top by 2
        RET                 ; Return
RNIS1   AND R5, R1, #1      ; Set R5 to 1 (Since R1 is -1)
        JSR OPRING          ; Operate
        BRnzp RRETN         ; Return
;
; Implement the PUT function
; The N is in R1 and will be saved into stack
;
PUT     STR R7, R2, #0      ; Save R7 to stack top
        STR R1, R2, #1      ; Save R1 to stack top
        ADD R2, R2, #2      ; Add stack top by 2
        ADD R1, R1, #-1     ; Subtract R1 by 1
        BRn PRETN           ; If N is 0 (now R1 is -1), return
        ADD R1, R1, #-1     ; Subtract R1 by 1
        BRn PNIS1           ; If N is 1 (now R1 is -1), jump to special process
        ADD R1, R1, #1      ; Add R1 by 1
        JSR PUT             ; PUT N - 1
        LDR R1, R2, #-1     ; Load R1 from stack
        ADD R1, R1, #-2     ; Subtract R1 by 2
        JSR REMOVE          ; REMOVE N - 2
        LDR R1, R2, #-1     ; Load R1 from stack
        JSR SHLEFT          ; Get 1 << (N - 1) to R5
        NOT R5, R5          ; Calculate -R5
        ADD R5, R5, #1
        JSR OPRING          ; Operate
        LDR R1, R2, #-1     ; Load R1 from stack
        ADD R1, R1, #-2     ; Subtract R1 by 2
        JSR PUT             ; PUT N - 2
PRETN   LDR R7, R2, #-2     ; Load the previous address from stack
        ADD R2, R2, #-2     ; Subtract stack top by 2
        RET                 ; Return
PNIS1   AND R5, R1, #-1     ; Set R5 to -1 (Since R1 is -1)
        JSR OPRING          ; Operate
        BRnzp PRETN         ; Return
;
; The initial pointer of stack
;
STKPTR  .FILL x6000
NADRES  .FILL x3100
OUTPTR  .FILL x3101
        .END
