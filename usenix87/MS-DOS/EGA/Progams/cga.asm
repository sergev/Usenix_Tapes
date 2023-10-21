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
MSG1    db      'EGA -> Color Emulator & Reboot',10,13,10,13,'$' ;startup msg
WAITMSG db      'Press any key to set CGA mode: $'               ;wait message
CGAMSG  db      ' *** CGA MODE NOW IN EFFECT ***',10,13,10,13,'$'
BOOTMSG db      'Press ENTER if you wish to boot a floppy now: $'
CRLF2   db      10,13,10,13,'$'
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
;       set up for CGA emulation
;******************************************************************************
 
        Mov     dx,offset MSG1          ;program startup message
        Mov     ah,09h                  ;print string
        Int     21h
 
        mov     ah,03h                  ;read cursor position
        int     10h
        push    bx                      ;save page number
        push    dx                      ;save row,column
 
        mov     al,INFO3
        and     al,0f0h                 ;set low nybble to 8
        or      al,08h
        mov     INFO3,al                ;set up for color emulation
        mov     ax,0082h                ;set normal text mode (performs the
        int     10h                     ;  actual mode switch)
 
        mov     ah,02h                  ;set cursor position
        pop     dx                      ;restore row,column
        pop     bx                      ;restore page number
        int     10h
        and     INFO,07fh               ;reset no clear screen bit (fix bug?)
 
;******************************************************************************
;       ask for and perform BOOT
;******************************************************************************
 
        Mov     dx,offset CGAMSG        ;tell about CGA mode
        Mov     ah,09h                  ;print string
        Int     21h
 
        Mov     dx,offset BOOTMSG       ;tell to press enter if desire boot
        Mov     ah,09h                  ;print string
        Int     21h
        Mov     ax,0C07h                ;flush buffer, console input w/o echo
        Int     21h
        push    ax
        Mov     dx,offset CRLF2         ;print 2 cariage returns
        Mov     ah,09h
        Int     21h
        pop     ax
        cmp     al,0dh                  ;compare to ENTER
        jnz     Exit
        Int     19h                     ;Dos boot interupt
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
