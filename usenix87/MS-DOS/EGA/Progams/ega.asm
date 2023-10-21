; (CTRL-OH)   IBM PC PRINTER CONDENSED MODE
        page    ,132
        title   colboot.asm
RET_FN  equ     4CH                     ;"return to dos" function call
 
;*********************************************************************
;
;       LOW MEMORY SEGMENT
;
;*********************************************************************
 
 
LowSeg  segment at 00h                  ;BIOS vectors & DOS variables
        org     487h
INFO    db      ?
        org     488h
INFO3   db      ?                       ;Feature bits, EGA DIP switches
LowSeg  Ends
 
 
;******************************************************************************
;
;       DATA SEGMENT
;
;******************************************************************************
 
 
Dseg    segment para public 'data'
MSG1    db      'EGA -> EGA mode',10,13,10,13,'$'             ;startup message
DONEMSG db      ' *** EGA MODE NOW IN EFFECT ***',10,13,'$'   ;ending message
RET_CD  db      0                                 ;errorlevel return code value
 
Dseg    Ends
 
 
;******************************************************************************
;
;       CODE SEGMENT
;
;******************************************************************************
 
 
Cseg    segment para public 'code'
        assume  cs:cseg,ss:stack        ;already set by dos loader
 
Entpt   proc    far                     ;entry point from dos
        Mov     ax,Dseg                 ;set up addressability to
        Mov     ds,ax                   ; the data segment
        assume  ds:Dseg
        Mov     ax,LowSeg               ;set up addressability to
        Mov     es,ax                   ; the low segment
        assume  es:LowSeg
 
;******************************************************************************
;       set up for EGA mode
;******************************************************************************
 
        Mov     dx,offset MSG1          ;program startup message
        Mov     ah,09h                  ;print string
        Int     21h
 
        mov     ah,03h                  ;read cursor position
        int     10h
        push    bx                      ;save page number
        push    dx                      ;save row,column
 
        mov     al,INFO3
        and     al,0f0h                 ;set low nybble to 9
        or      al,09h
        mov     INFO3,al                ;set up for enhanced color display
        mov     ax,0082h                ;set normal text mode (performs the
        int     10h                     ;  actual mode switch - no cls.)
 
        mov     ah,02h                  ;set cursor position
        pop     dx                      ;restore row,column
        pop     bx                      ;restore page number
        int     10h
        and     INFO,07fh               ;reset no clear screen bit (fix bug?)
 
        Mov     dx,offset DONEMSG       ;tell about EGA mode now
        Mov     ah,09h                  ;print string
        Int     21h
        jmp     Exit
 
 
;******************************************************************************
;       RETURN TO DOS
;******************************************************************************
 
 
Exit:   Mov     ah,RET_FN               ;return to dos function call, and
        Mov     al,RET_CD               ;value to be passed to errorlevel
        Int     21H                     ;return to dos
Entpt   Endp                            ; (version 2.00 or later)
Cseg    ENDS
 
STACK   SEGMENT PARA STACK 'STACK'
DB      64 DUP("STACK   ")      ;256 WORD STACK AREA
STACK   ENDS
END     ENTPT
