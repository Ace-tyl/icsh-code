        .ORIG x3000
;
; Input password first time
;
        AND R6, R6, #0          ; Clear R6
        JSR READ_S3             ; Read input to S3
        LD R4, S1_ADDR          ; Load the address of S1 to R4
        JSR STRCMP              ; Compare two strings
        BRp RQUIT               ; If correct then puts S4 and quit
        LD R0, S5_ADDR          ; Load the address of S5 to R0 ("wron\n")
        PUTS                    ; Output S5
;
; Input verification code
;
        LD R4, S2_ADDR          ; Load the address of S2 to R4 (Verification code)
        ADD R0, R4, #0          ; Load the address of S2 to R0 (Verification code)
        PUTS                    ; Output the verification code
        AND R0, R0, #0          ; Clear R0
        ADD R0, R0, #10         ; Set R0[7:0] to '\n'
        OUT                     ; Print this '\n'
        ADD R6, R6, #1          ; Add R6 by 1
        JSR READ_S3             ; Read input to S3
        JSR STRCMP              ; Compare S3 with S2
        BRz WQUIT               ; If incorrect then puts S5 and quit
        LD R0, S4_ADDR          ; Load the address of S4 to R0 ("righ\n")
        PUTS                    ; Output S4
;
; Input password second time
;
        AND R6, R6, #0          ; Clear R6
        JSR READ_S3             ; Read input to S3
        LD R4, S1_ADDR          ; Load the address of S1 to R4
        JSR STRCMP              ; Compare two strings
        BRp RQUIT               ; If correct then puts S4 and quit
WQUIT   LD R0, S5_ADDR          ; Load the address of S5 to R0 ("wron\n")
        PUTS                    ; Output S5
        HALT                    ; End the program
RQUIT   LD R0, S4_ADDR          ; Load the address of S4 to R0 ("righ\n")
        PUTS                    ; Output S4
        HALT                    ; End the program
;
; Function READ_S3
; Read string to S3
; Parameters: R6 is whether the input will be displayed (If R6 is not zero then will display)
; This method will use register R0, R5 and R6
;
READ_S3 LD R5, S3_ADDR          ; Load the string address to R5
LOOPRS3 GETC                    ; Read a character
        STR R0, R5, #0          ; Store the character to memory
        ADD R5, R5, #1          ; Move the pointer forward 1
        ADD R6, R6, #0          ; Read the value in R6
        BRz NODISP              ; Don't print the character
        OUT                     ; Print the character
NODISP  ADD R0, R0, #-16        ; Subtract R0 by 16 to judge if the character is '\n', '\r' or '\0'
        BRp LOOPRS3             ; If the string is not ended then go to next loop
        AND R0, R0, #0          ; Clear R0 to obtain '\0'
        STR R0, R5, #-1         ; Store '\0' to the last character of the string
        RET                     ; Return
;
; Function STRCMP
; Compare string S3 with string address R4
; Parameters: R4 is the comparing string address
; Returns: R3 is the result whether two strings are equal (1 is equal and 0 otherwise)
; This method will use register R0, R3, R4, R5
;
STRCMP  LD R5, S3_ADDR          ; Load S3 string address to R5
LOOPSC  LDR R0, R4, #0          ; Load the character of first string to R0
        LDR R3, R5, #0          ; Load the character of second string to R3
        NOT R3, R3              ; Calculate R3 = -R3 (Step 1)
        ADD R3, R3, #1          ; Calculate R3 = -R3 (Step 2)
        ADD R3, R3, R0          ; Add R3 with R0 to compare R0 and R3
        BRnp NOTEQ              ; Two strings not equal
        ADD R4, R4, #1          ; Move pointer R4 forward 1
        ADD R5, R5, #1          ; Move pointer R5 forward 1
        ADD R0, R0, #0          ; Judge if R0 is '\0'
        BRp LOOPSC              ; If the string has not ended then go to next loop
        ADD R3, R0, #1          ; Set R3 to 1 (Since R0 must be 0)
        RET                     ; Return
NOTEQ   AND R3, R3, #0          ; Set R3 to 0
        RET                     ; Return
;
; Addresses of strings
;
S1_ADDR .FILL x3100
S2_ADDR .FILL x3200
S3_ADDR .FILL x3300
S4_ADDR .FILL x3400
S5_ADDR .FILL x3500
        .END

        .ORIG x3100
S1      .STRINGZ "hello"        ; Password
        .END
        .ORIG x3200
S2      .STRINGZ "world"        ; Verification Code
        .END
        .ORIG x3400
S4      .STRINGZ "righ\n"       ; String "righ\n"
        .END
        .ORIG x3500
S5      .STRINGZ "wron\n"       ; String "wron\n"
        .END
