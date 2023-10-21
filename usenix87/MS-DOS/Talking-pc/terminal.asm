
;------------------------------------------------------------------------------
;	Talking console device driver for the AT&T PC6300.
;	Written by Karl Dahlke, September 1986.
;	Property of AT&T Bell Laboratories, all rights reserved.
;	This software is in the public domain and may be freely
;	distributed to anyone, provided this notice is included.
;	It may not, in whole or in part, be incorporated in any commercial
;	product without AT&T's explicit permission.
;------------------------------------------------------------------------------

;	terminal.asm: dumb terminal emulator

;	little more than a bidirectional cat running at interupt level.
;	given the talking device driver, I neither need nor want more.

;	must be a .com program, run exe2bin.

BUFSZ	equ 256 ; size of input and output circular buffers
UDSZ	equ 1024 ; size of upload/download temp buffer
STKSZ	equ 256 ; size of the stack

XOFF	equ 19 ; flow control supported in both directions
XON	equ 17

;	terminal can be on com1 or com2
COM	equ 2

RSBASE equ 4f0h-COM*100h    ;base of address of aux. port registers
DATREG equ RSBASE + 8     ;data register
LDL equ RSBASE + 8        ;low divisor latch
HDL equ RSBASE + 9        ;high divisor latch
IER equ RSBASE + 9        ;interrupt enable register
IIR equ RSBASE + 0ah        ;interrupt identification register
LCR equ RSBASE + 0bh        ;line control register
MCR equ RSBASE + 0ch        ;modem control register
LSR equ RSBASE + 0dh        ;line status register
MSR equ RSBASE + 0eh        ;modem status register


;	buffers follow text, in uninitialized memory
rxbuf	equ offset PGROUP: udata+0
txbuf	equ offset PGROUP: udata+BUFSZ
udarea	equ offset PGROUP: udata+BUFSZ+BUFSZ
maxmem	equ offset PGROUP: udata+BUFSZ+BUFSZ+UDSZ+STKSZ


PGROUP  group   PROG, DATA, UDATA
	assume  cs:PGROUP, ds:PGROUP


PROG    segment para public 'PROG'

	org 100h

term	proc near
;	is there enough memory?
	sti
	cmp sp,maxmem-2
	jae memok
	mov ah,9
	mov dx,offset PGROUP: nmm
	int 21h
	mov ax,4c01h
	int 21h
	memok:

	cli
	cld
	xor ax,ax
	mov ds,ax

;	init the serial port
	mov dx,MCR
	mov al,0
	out dx,al
	mov dx,LSR         ;reset line status
	in al,dx
	mov dx,DATREG      ; clear any input
	in al,dx
	mov dx,MSR
	in al,dx

;	set baud rate to 1200 baud
	mov dx,LCR
	mov al,83h ; access divisor latch
	out dx,al
	mov dx,LDL
	mov al,96       ;low byte of divisor
	out dx,al
	mov dx,HDL
	mov al,0       ;high byte of divisor
	out dx,al
	mov dx,LCR
	mov al,3 ; 8 bits, no parity
	out dx,al

;	set interrupt vector
	mov word ptr [bx+34h-COM*4],offset PGROUP: ih_com
	mov [bx+36h-COM*4],cs

	mov dx,IER                 ;enable interrupts on 8250
	mov al,1 ; only receive interrupt initially
	out dx,al

	in al,21h                     ;set enable bit on 8259
	and al,0e7h+COM*8 ; bit 4/3 is serial interrupt
	nop ; timing
	nop
	out 21h,al

	mov dx,MCR
	mov al,7+COM*4 ; dtr and rts set
;	extra bit (8[4]) is needed to generate interrupts.
	out dx,al


	push cs
	pop ds

main:	; infinite loop
	sti
;	receive buffer to screen
	mov bx,rxtail
	cmp bx,rxhead
	jnz input
	jmp noinp
input:	mov al,[bx]
	and al,7fh
	mov bx,indl ; in download?
	or bx,bx
	jz nodl
	cmp al,26 ; eof
	jz wrfl
	mov [bx],al
	inc bx
	mov indl,bx
	cmp bx,udarea+UDSZ
	jb incrx
wrfl:	push ax
	mov ah,40h
	mov cx,bx
	mov bx,udfd
	mov dx,udarea
	sub cx,dx
	jz zlen
	int 21h ; write block
	jnc z1len
badwr:	mov dx,offset PGROUP: cwm
	mov ah,9
	int 21h ; display string
	pop ax
	jmp short dldone
z1len:	cmp ax,cx
	jnz badwr

zlen:	mov indl,udarea
	mov dl,'.'
	mov ah,6
	int 21h ; output '.'
	pop ax
	cmp al,26
	jnz incrx
dldone:	mov indl,0
	mov bx,udfd
	mov ah,3eh
	int 21h ; close file
	mov udfd,0
	jmp short incrx

nodl:	mov ah,6
	mov dl,al
	int 21h ; display character
incrx:	mov bx,rxtail
	inc bx
	cmp bx,rxbuf+BUFSZ
	jnz wrap4
	mov bx,rxbuf
wrap4:	mov rxtail,bx
	mov dx,rxhead
	sub dx,bx
	jae wrap6
	add dx,BUFSZ
wrap6:	cmp dx,50
	jnz noinp
	cli
	cmp sentxf,0
	jz noinp
	mov flow,XON
	mov sentxf,0
	jmp begx1

;	from keyboard to transmit buffer
noinp:	sti
	mov bx,inul
	or bx,bx
	jz g1
	cmp bx,ultop
	jb nr1
	mov ah,3fh
	mov bx,udfd
	or bx,bx
	jnz ulmore ; udfd set to 0 if the last read reached eof
	cmp lastc,26 ; if we sent a control-z, send an erase character
	jnz nocth
	mov bx,udarea
	mov byte ptr [bx],8
	mov inul,bx
	inc bx
	mov ultop,bx
nr1:	jnz noread
nocth:	mov ah,6
	mov dl,13
	int 21h ; display cr
	mov ah,6
	mov dl,10
	int 21h ; display nl
	mov inul,0
	jmp short g1
ulmore:	mov dx,udarea
	mov cx,UDSZ
	int 21h ; read block
	jnc rdok
	mov ah,3eh
	int 21h ; close file
	mov udfd,0
	mov ah,9
	mov dx,offset PGROUP: crm
	int 21h ; display string
	mov inul,0
g1:	jmp short getkey
rdok:	cmp ax,cx
	jz noeof
	push ax
	mov ah,3eh
	int 21h ; close file
	mov udfd,0
	pop ax
noeof:	mov bx,udarea
	mov inul,bx
	add ax,bx
	mov ultop,ax
	mov ah,6
	mov dl,'.'
	int 21h ; display '.'
	jmp short getkey ; in case 0 bytes read
noread:	mov bx,txtail
	cmp bx,txhead
	jnz getkey
	mov bx,inul
	mov al,[bx]
	mov lastc,al
	inc bx
	mov inul,bx
	cmp al,10
	jnz nolf
;	a linefeed, don't send it to the host computer
;	in addition, we delay here.
;	for some reason, the vax needs this pause after cr, to digest the line
	mov cx,8000h
	loop $
	jz getkey
nolf:	jmp outal

getkey:	mov dl,0ffh
	mov ah,6
	int 21h ; getchar
	jz l1 ; no keyboard characters
	cmp al,0
	jz alt
	jmp noalt
alt:	mov dl,0ffh
	mov ah,6
	int 21h ; getchar

	cmp al,26h
	jz exit
	cmp al,2dh
	jnz n2d
;	hang up, drop dtr
	mov dx,MCR
	mov al,0
	out dx,al

exit:	in al,21h
	or al,24-COM*8
	nop
	nop
	out 21h,al
	mov bx,udfd
	or bx,bx
	jz noopen
	mov ah,3eh
	int 21h ; close file
noopen:	mov ax,4c00h
	int 21h ; goodbye

n2d:	cmp al,16h
	jnz n16
;	upload
	call fname
	jz l1
	mov ax,3d00h
	int 21h ; open file
	jnc ok2
	mov dx,offset PGROUP: cam
	jc out1st
ok2:	mov udfd,ax
	mov ax,udarea
	mov inul,ax
	mov ultop,ax
	mov lastc,0
l1:	jmp main

n16:	cmp al,20h
	jnz n20
;	download
	call fname
	jz l1
	mov ah,3ch
	xor cx,cx
	int 21h ; create file
	jnc ok1
	mov dx,offset PGROUP: ccm
	jc outstr
ok1:	mov udfd,ax
	mov indl,udarea
	jmp main

n20:	cmp al,13h
	jnz n13
;	toggle baud rate
	mov dx,LCR
	cli
	mov al,83h ; access divisor latch
	out dx,al
	mov dx,LDL
	mov ax,480
	sub ax,curbaud
	mov curbaud,ax
	out dx,al
	mov dx,HDL
	mov al,ah       ;high byte of divisor
	out dx,al
	mov dx,LCR
	mov al,3 ; 8 bits, no parity
	out dx,al
	sti
	mov dx,offset PGROUP: baud12
	or ah,ah
	jz outstr
	mov dx,offset PGROUP: baud3
out1st:	jmp short outstr

n13:	cmp al,1fh
	jnz n1f
;	report status: dsr, carrier, and cts
	mov dx,MSR
	in al,dx
	mov dl, '-'
	test al,20h
	jz nodsr
	mov dl,'a'
nodsr:	mov swm+2,dl
	mov dl, '-'
	test al,80h
	jz nocar
	mov dl,'c'
nocar:	mov swm+3,dl
	mov dl, '-'
	test al,10h
	jz nocts
	mov dl,'s'
nocts:	mov swm+4,dl
	mov dx,offset PGROUP: swm
outstr:	mov ah,9
	int 21h ; display string
	jmp main

n1f:	cmp al,30h
	jnz n30
;	send break
	mov dx,LCR         ;save the line control register
	in al,dx
	or al,40h ; set break condition
	nop
	out dx,al
;	wait a while, in break
	mov cx,0ffffh
	loop $
	and al,0bfh
	out dx,al
n30:	jmp main

noalt:	cmp inul,0 ; send no keyboard characters if uploading
	jnz n30
outal:	mov bx,txhead
	mov dx,bx ; save
	mov [bx],al
	inc bx
	cmp bx,txbuf+BUFSZ
	jnz wrap5
	mov bx,txbuf
wrap5:	cmp bx,txtail
	jz n30 ; buffer full, character not sent
	cli
	mov txhead,bx
	cmp dx,txtail
	jnz n30
begx1:	cmp noxmit,0
	jnz n30
	mov dx,IER
	mov al,3
	out dx,al
	jmp main


term	endp


;	interrupt handler for serial interrupts
ih_com	proc far
	cld
	push ds
	push cs
	pop ds
	push ax
	push bx
	push dx

repoll:
;	When determining what actions to take, I find the IIR nigh unto
;	useless.  We have to look at LSR and MSR anyways, 
;	so why bother with IIR?
;	we read IIR, only because the interrupt system requires it.
	mov dx,IIR
	in al,dx
	nop
	nop
	mov dx,LSR
	in al,dx
	test al,1
	jnz rxint
	test al,20h
	jnz txint

intend:	mov al,65h-COM
	out 20h,al
	pop dx
	pop bx
	pop ax
	pop ds
	iret

txint:	mov dx,DATREG
	mov bx,txtail
	cmp noxmit,0
	jnz intend
	mov al,flow
	cmp al,0
	jnz send
	cmp bx,txhead
	jz intend ; nothing to transmit
	mov al,[bx]    ;get data from buffer
	inc bx
	cmp bx,txbuf+BUFSZ
	jnz wrap1
	mov bx,txbuf
wrap1:	mov txtail,bx
send:	out dx,al ; send data
	mov flow,0

	;if no more data to send then reset tx interrupt and return
	cmp bx,txhead
	jnz intend
stopx:	mov dx,IER
	mov al,1
	out dx,al
	jmp short intend

rxint:	mov dx,DATREG ; get input byte
	in al,dx
	cmp al,XOFF
	jnz noxoff
	inc noxmit
	jnz stopx
noxoff:	cmp al,XON
	jnz noxon
	mov noxmit,0
	mov bx,txtail
	cmp bx,txhead
	jz repoll
begx:	mov dx,IER
	mov al,3
	out dx,al
r1:	jmp repoll

noxon:	mov bx,rxhead
	mov [bx],al
	inc bx
	cmp bx,rxbuf+BUFSZ
	jnz wrap2
	mov bx,rxbuf
wrap2:	cmp bx,rxtail
	jz r1 ; sorry, buffer full, throw character away
	mov rxhead,bx
	sub bx,rxtail ; how full is receive buffer
	jae wrap3
	add bx,BUFSZ
wrap3:	cmp bx,BUFSZ-50
	jnz r1
	cmp sentxf,0
	jnz r1
	mov flow,XOFF
	inc sentxf
	cmp noxmit,0
	jz begx
	jnz r1

ih_com	endp

;	get filename to be transfered
fname	proc near
	mov ah,9
	mov dx,offset PGROUP: fnm
	int 21h ; display string
;	get file name using MS-DOS getline function call
	mov ah,0ah
	mov dx,offset PGROUP: fnlim
	int 21h ; get string
	mov bh,0
	mov bl,fnlen
	mov fnstr[bx],0 ; ASCII Z string
	mov dx,offset PGROUP: fnstr
	or bl,bl
	ret
fname	endp

PROG	ends

DATA    segment para public 'DATA'

indl	dw 0
inul	dw 0
ultop	dw 0
udfd	dw 0 ; file descripter

rxhead	dw rxbuf
rxtail	dw rxbuf
txhead	dw txbuf
txtail	dw txbuf

fnlim	db 50
fnlen	db 0
fnstr	db 50 dup(?)

curbaud	dw 96
noxmit	db 0
sentxf	db 0
flow	db 0
lastc	db 0

swm	db 13,10,"---",13,10,36
baud12	db 13,10,"1200 baud",13,10,36
baud3	db 13,10,"300 baud",13,10,36
fnm	db "file name? ",36
cam	db "cannot open file",13,10,36
ccm	db "cannot create file",13,10,36
crm	db 13,10,"cannot read from file",13,10,36
cwm	db 13,10,"cannot write to file",13,10,36
nmm	db "insufficient memory",13,10,36

DATA	ends

UDATA    segment para public 'UDATA'
UDATA	ends

	end term

