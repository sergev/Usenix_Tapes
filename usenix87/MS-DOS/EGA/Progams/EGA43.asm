Here is a sample program to put the EGA in 43 line mode. It is also on
IX/370 under /u1/wheeler.
    
PAGE    60,132
TITLE MODE43 - Put EGA with ECD into 43 line mode
;
DATA    SEGMENT AT 40H
        ORG     88H
EGAMODE DB      ?
DATA    ENDS
;
CODE    SEGMENT PARA PUBLIC 'CODE'
        ASSUME  CS:CODE,DS:CODE
;
MODE43  PROC    NEAR
;
        MOV     AX,0F00H        ; Get current video mode
        INT     10H
        XOR     AH,AH           ; Now do mode set
        INT     10H
        MOV     AX,1112H        ; Select 8x8 double dot font
        MOV     BL,0
        INT     10H
        MOV     AX,DATA         ; Change special EGA byte to get cursor
        MOV     DS,AX
        ASSUME  DS:DATA
        MOV     EGAMODE,0F8H    ; Set to allow 8x8 cursor
        MOV     AX,0100H        ; Set cursor type
        MOV     CX,0607H        ; Select appropriate cursor
        INT     10H
        MOV     EGAMODE,0F9H    ; Set back
        PUSH    CS              ; Set DS back to CS
        POP     DS
        ASSUME  DS:CODE
        MOV     AX,1200H        ; Select alternate print screen routine
        MOV     BL,20H
        INT     10H
        INT     20H             ; Return to DOS
MODE43  ENDP
;
CODE    ENDS
;
        END     MODE43
