
;------------------------------------------------------------------------------
;	Talking console device driver for the AT&T PC6300.
;	Written by Karl Dahlke, September 1986.
;	Property of AT&T Bell Laboratories, all rights reserved.
;	This software is in the public domain and may be freely
;	distributed to anyone, provided this notice is included.
;	It may not, in whole or in part, be incorporated in any commercial
;	product without AT&T's explicit permission.
;------------------------------------------------------------------------------

;	synth.asm: speech synthesizer routines for talking device driver

;	currently designed to control a Votrax Type & Talk speech unit
;	via com1
;	application programs should not use com1 when this driver is installed
;	we use an interrupt driven system, bypassing bios.

	include parms.h

;	synthesizer can be on com1 or com2
COM	equ 1

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


PGROUP	group	PROG, DATA
	assume cs:PGROUP, ds:PGROUP

DATA	segment	word public 'DATA'

	extrn rdflag:byte, inspeech:byte
	extrn buzzfc:byte

xmit_ptr dw 0 ; pointer to the next character to be transmitted
novo	db 0 ; no votrax present
crticks	db 0 ; number of ticks to wait after sending return
inxmit	db 0 ; flag indicating transmission in progress

DATA	ends


PROG	segment	byte public 'PROG'

	extrn putfifo:near
	public ss_ready, ss_text, ss_shutup
	public ss_init

;	init synthesizer, in this case the serial port
ss_init	proc near
;	reset the UART
	mov dx,MCR
	mov al,0
	out dx,al
	mov dx,LSR         ;reset line status
	in al,dx
	mov dx,DATREG      ; clear any input
	in al,dx
	mov dx,MSR
	in al,dx

;	set baud rate to 9600 baud
	mov dx,LCR
	mov al,80h ; access divisor latch
	out dx,al
	mov dx,LDL
	mov al,12       ;low byte of divisor
	out dx,al
	mov dx,HDL
	mov al,0       ;high byte of divisor
	out dx,al

	mov dx,LCR
	mov al,3 ; 8 bits, no parity
	out dx,al

;	set interrupt vector
	mov word ptr [bx+34h-COM*4],ADDR: ih_com
	mov [bx+36h-COM*4],cs

	mov dx,IER                 ;enable interrupts on 8250
	mov al,0 ; no transmit interrupt initially
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

	ret
ss_init	endp

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
	test al,20h
	jnz txint

intend:	mov al,65h-COM
	out 20h,al
	pop dx
	pop bx
	pop ax
	pop ds
	iret

	;if no data to send then reset tx interrupt and return
txint:	mov bx,xmit_ptr
	or bx,bx
	jnz oktx
	mov dx,IER
	mov al,0
	out dx,al
	dec inxmit
	jmp intend

oktx:	mov al,[bx]    ;get data from buffer
	inc bx
	mov dx,DATREG
	or al,al
	jnz more
;	end of string, sen return byte
	mov al,13
	xor bx,bx
more:	out dx,al                     ;send data
	mov xmit_ptr,bx
	jmp intend

ih_com	endp

;	return status of synthesizer,  ready for more or not
;	ideally, the unit is ready when it has spoken everything
;	previously sent to it.  the Type & Talk has an
;	annoying delay of about 10 seconds between ready indicated
;	and all input spoken.  There is nothing I can do
;	about this.  Other units, such as the echo,
;	are out of the question, since their delay is measured
;	in minutes.  Someday, a speech unit will indicate clearly when
;	it has spoken all input.
ss_ready proc near
;	sending a string?
	cmp inxmit,0
	jnz ss_busy
;	a real time delay, after sending return
	cmp crticks,0
	jz crok
	dec crticks
ss_busy:	xor ax,ax
	ret
crok:
	mov dx,MSR
	in al,dx
;	check to see if votrax is even there (dsr)
	test al,20h
	jnz alive
	cmp novo,0
	jnz nobuzz
	inc novo
	mov rdflag,0
	mov inspeech,0
	mov bx,ADDR: buzzfc
	call putfifo
nobuzz:	or al,10h
alive:	and ax,10h ; clear to send indicates Votrax is ready
	ret
ss_ready endp

;	send text string to the speech synthesizer
;	routine assumes the text is to be spoken immediately
;	once the text is spoken, cts will indicate ready for more
ss_text	proc near
	mov novo,0
	mov xmit_ptr,bx ; set pointer to buffer
;	set delay, after text is transmitted, so votrax can disable cts
	inc crticks
	inc inxmit ; in transmit state
;	enable transmit register empty interrupt, to transmit the text
	mov dx,IER
	mov al,2
	out dx,al
	ret
ss_text endp

;	shut up, and discard accumulated text
;	annoyingly, type & talk must be "ready" for text,
;	before it will honor the shut up command (break on rs232).
ss_shutup	proc near
;	wait a few miliseconds, votrax needs this
	sti
	mov cx,62*CLKRATE
	loop $
	cli
	mov dx,LCR         ;get the line control register
	in al,dx
	or al,40h       ;set break condition
	nop
	out dx,al
;	wait a while, in break
;	sorry to take real time for this, but, it is easier
	mov cx,57*CLKRATE
	loop $
	and al,0bfh
	out dx,al
	sti
	ret
ss_shutup	endp

PROG	ends

	end

