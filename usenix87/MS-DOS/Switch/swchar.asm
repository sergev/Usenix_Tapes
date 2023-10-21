	name	swchar
	page	55,132
	title	'SWCHAR --- display or change switch character'
;
; Copyright 1984 by Michael D. Parker (mike@LOGICON)
; This program may be freely reproduced and modified for
; noncommercial use.
;
; ---------- Explanation

; This program that will either report or set the SWITCHAR from the DOS
; command level. Use for version of DOS 2.00 and LATER.

; Command line calling sequence
;
;	swchar		      (to report current switchar)
;
;	   or
;
;	swchar x      (to change switchar to 'x')
;

; This program uses the SWITCHAR function of the DOS function request
; interrupt.  This DOS function, 37H is listed by IBM as "used
; internally by DOS".  It is passed either 1 or 2 parameters,
; depending on usage.

; To get the current SWITCHAR, put 37H in the AH register, 00H in the
; AL register, and generate an INT 21H. The current SWTICHAR will be
; returned in the DL register.

; To change the current SWITCHAR, put 37H in the AH register, 01H in
; the AL register, the new SWITCHAR in the DL register and generate an
; INT 21H.

; first the program equ statements

cr	equ	0dh		;ASCII carriage return
lf	equ	0ah		;ASCII line feed
eom	equ	'$'             ;end of message flag
				;Program Segment Prefix:
command equ	80h		;command line buffer
doscal	equ	21h		;code for dos interrupt for processing
swread	equ	3700h		;DOS call for reading switchar
swset	equ	3701h		;DOS call for setting switchar
outstr	equ	9h		;DOS call for output string

;
; CODE segment starts here
;
cseg	segment para public 'CODE'
	assume	cs:cseg,ds:data,es:data,ss:stack
;
; first check to see if proper version of DOS to execute this item
; for
;
swchar	proc	far		;entry point from PC-DOS
	push	ds		;save DS:0000 for final
	xor	ax,ax		;return to PC-DOS
	push	ax
;
; now check for parameters on the command line
;
	mov	ax,data 	;make our data segment addressable
	mov	es,ax		;via the ES register
	mov	si,offset command
	lodsb			;check string length byte,
	or	al,al		;any char present?
	jz	swc200		;no,then get current switchar
;
; examine command line, and stop if non-blank character found
; or end of line (cr)
;
swc150: 			;scan for start of char
	lodsb			;get next char
	cmp	al,cr		;if carriage return,char is missing
	je	swc200		;so jump to get current switchar
	cmp	al,' '          ;if blank, keep looking
	je	swc150
;
; a new character for the switchar
;
	mov	es:chrx,al	;save the character
	mov	ax,es		;now set DS=ES for remainder of program
	mov	ds,ax
	mov	dl,ds:chrx	;get the character for new switchar
	mov	ax,swset	; change the SWITCHAR
	int	doscal		;perform DOS call
	jmp	swc250		;tell the abuser what the char is now
;
; find the current switchar and print it out
;
				;let ES:DI = addr of name field
swc200:
	mov	ax,es		;now set DS=ES for remainder
	mov	ds,ax		;of the program.
swc250:
	mov	ax,swread	; (yes)-read SWITCHAR
	int	doscal		;call DOS to read switchar
	mov	byte ptr curch,dl  ; prepare report string
	mov	ah,outstr	; print report string
	mov	dx,offset report
	int	doscal		;make DOS call
	ret			;all done-- then return to DOS
swchar	endp

cseg	ends
;
; DATA segment starts here
;

data	segment para public 'DATA'

report	db	'SWITCHAR is '  ;Tell the user what current char is
curch	db	'X'             ;space for current switchar
	db	cr,lf,eom	;message terminator

chrx	db	0		;temp storage

data	ends

;
; STACK Segment starts here
;
stack	segment para stack 'STACK'
	db	64 dup (?)	;just something to keep things buzy
stack	ends
	end	swchar

