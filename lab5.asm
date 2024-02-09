; Unfortunately we have not YET installed Windows or Linux on the LC-3,
; so we are going to have to write some operating system code to enable
; keyboard interrupts. The OS code does three things:
;
;    (1) Initializes the interrupt vector table with the starting
;        address of the interrupt service routine. The keyboard
;        interrupt vector is x80 and the interrupt vector table begins
;        at memory location x0100. The keyboard interrupt service routine
;        begins at x1000. Therefore, we must initialize memory location
;        x0180 with the value x1000.
;    (2) Sets bit 14 of the KBSR to enable interrupts.
;    (3) Pushes a PSR and PC to the system stack so that it can jump
;        to the user program at x3000 using an RTI instruction.

            .ORIG   x800
            ; (1) Initialize interrupt vector table.
            LD      R0, VEC
            LD      R1, ISR
            STR     R1, R0, #0
    
            ; (2) Set bit 14 of KBSR.
            LDI     R0, KBSR
            LD      R1, MASK
            NOT     R1, R1
            AND     R0, R0, R1
            NOT     R1, R1
            ADD     R0, R0, R1
            STI     R0, KBSR
    
            ; (3) Set up system stack to enter user space.
            LD      R0, PSR
            ADD     R6, R6, #-1
            STR     R0, R6, #0
            LD      R0, PC
            ADD     R6, R6, #-1
            STR     R0, R6, #0
            ; Enter user space.
            RTI
        
VEC         .FILL   x0180
ISR         .FILL   x1000
KBSR        .FILL   xFE00
MASK        .FILL   x4000
PSR         .FILL   x8002
PC          .FILL   x3000
            .END



            .ORIG   x3000
            ; *** Begin user program code here ***
            LD      R6, USP             ; initialize USP
            
PRINTID     LDI     R1, N_ADDR          ; Read value N
            BRzp    FACTORIAL           ; If N is a number then start calculate
            LEA     R0, STU_ID          ; Get the address of student ID
            PUTS                        ; Print the student ID
            JSR     DELAY               ; Delay
            BR      PRINTID

FACTORIAL   ADD     R0, R1, #-7         ; Judge if N is too large
            BRp     TOOLAREX            ; Greater than 7 means too large
            ADD     R2, R1, #0          ; Set R2 to N
            AND     R4, R4, #0          ; Clear R4
            ADD     R4, R4, #1          ; Set R4 to 1
FACLOOP     AND     R5, R5, #0          ; Clear R5
            ADD     R3, R2, #0          ; Copy R2 to R3
FACSUBLP    ADD     R5, R5, R4          ; Add R5 by R4 (to calculate R4 * R2)
            ADD     R3, R3, #-1         ; Subtract R3 by 1
            BRp     FACSUBLP            ; Loop R2 times
            ADD     R4, R5, #0          ; Copy R5 to R4
            ADD     R2, R2, #-1         ; Subtract R2 by 1
            BRp     FACLOOP             ; Loop N times
            ; Now the result is stored in R4
            ; Then the result will be converted to decimal
            LEA     R5, RESPTR
DLOOP       AND     R3, R3, #0          ; Clear R3 (used to store the divided value)
DIVLOOP     ADD     R3, R3, #1          ; Add R3 by 1
            ADD     R4, R4, #-10        ; Subtract R4 by 10
            BRzp    DIVLOOP             ; If non-negative then go to next loop
            ADD     R4, R4, #10         ; Add R4 by 10
            ADD     R4, R4, #12
            ADD     R4, R4, #12
            ADD     R4, R4, #12
            ADD     R4, R4, #12         ; Add R4 by 48
            STR     R4, R5, #-1         ; Store R4 to address R5 - 1
            ADD     R5, R5, #-1         ; Subtract R5 by 1
            ADD     R4, R3, #-1         ; Subtract R3 by 1
            BRp     DLOOP               ; Get the next digit
            ; Print the result
            ADD     R0, R1, #12
            ADD     R0, R0, #12
            ADD     R0, R0, #12
            ADD     R0, R0, #12         ; Set R0 to R1 + 48
            OUT                         ; Print this number
            LEA     R0, FACEQ
            PUTS                        ; Print "! = "
            ADD     R0, R5, #0          ; Set address to R5
            PUTS                        ; Print the result number
            LEA     R0, FACEND
            PUTS                        ; Print the end.
            HALT

TOOLAREX    ADD     R0, R0, #13
            ADD     R0, R0, #14
            ADD     R0, R0, #14
            ADD     R0, R0, #14         ; Add R0 by 48 + 7
            OUT                         ; Print this number
            LEA     R0, TOOLARGE        ; Get the string
            PUTS                        ; Print the string
            HALT
            
DELAY       ST      R1, SaveR1
            LD      R1, Count
REP         ADD     R1, R1, #-1
            BRp     REP
            LD      R1, SaveR1
            RET
SaveR1      .BLKW   #1
Count       .FILL   #2500               ; Loop times

N_ADDR      .FILL   FACT_N
STU_ID      .STRINGZ "PB01234567 "      ; Student ID
TOOLARGE    .STRINGZ "! is too large for LC-3.\n"
FACEQ       .STRINGZ "! = "
FACEND      .STRINGZ ".\n"
            .BLKW   #10                 ; A temporary space to store the result
RESPTR      .BLKW   #1

USP         .FILL   xFE00
            ; *** End user program code here ***
            .END



            .ORIG   x3FFF
            ; *** Begin factorial data here ***
FACT_N      .FILL   xFFFF
            ; *** End factorial data here ***
            .END



            .ORIG   x1000
            ; *** Begin interrupt service routine code here ***
            STI     R0, R0STORE         ; Store the original value of R0
            STI     R1, R1STORE         ; Store the original value of R1
            AND     R0, R0, #0
            ADD     R0, R0, #10         ; Set R0 to '\n'
            OUT                         ; Print a line break
            LDI     R0, KBDR            ; Read the key
            ADD     R1, R0, #-16
            ADD     R1, R1, #-16
            ADD     R1, R1, #-16        ; Subtract R0 by 48 to R1
            BRn     NOTNUM              ; Not a number
            ADD     R1, R1, #-9         ; Subtract R1 by 9
            BRp     NOTNUM              ; Not a number
            OUT                         ; Putchar
            ADD     R1, R1, #9          ; Add R1 by 9
            STI     R1, NN_ADDR         ; Store R1 to x3FFF
            LEA     R0, ISDEC           ; Load the string
            PUTS                        ; Print the string
            BR      COMPLETE

NOTNUM      OUT                         ; Putchar
            LEA     R0, ISNTDEC         ; Load the string
            PUTS                        ; Print the string
            
COMPLETE    LDI     R0, R0STORE         ; Load the value in R0
            LDI     R1, R1STORE         ; Load the value in R1
            RTI
            
NN_ADDR     .FILL   FACT_N
KBDR        .FILL   xFE02
R0STORE     .BLKW   #1
R1STORE     .BLKW   #1
ISNTDEC     .STRINGZ " is not a decimal digit.\n"
ISDEC       .STRINGZ " is a decimal digit.\n"
            ; *** End interrupt service routine code here ***
            .END
