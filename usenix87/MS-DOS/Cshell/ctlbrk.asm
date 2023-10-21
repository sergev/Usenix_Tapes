; Copyright (C) 1985 by Manx Software Systems, Inc.
; :ts=8
	include lmacros.h

dataseg	segment	word public 'data'
	extrn	_PSP_:word
	_brkvec	dw ?
		dw ?
	localstk	dw	20	dup(?)
	stktop	label	word
	savess	dw ?
	savesp	dw ?
dataseg	ends

ourds	dw	0

	assume	ds:dataseg
;
	procdef	ctl_brk_setup
	mov	ourds,ds
	push	ds
	mov	ax,3523H	;get cntl-break (cntl-c) handler
	int	21H
	mov	_brkvec,bx
	mov	_brkvec+2,es
	mov	dx,offset brk_handler
	mov	ax,cs
	mov	ds,ax
	mov	ax,2523H	;set new cntl-break handler
	int	21H
	pop	ds
	pret
	pend	ctl_brk_setup

	procdef	ctl_brk_restore
	push	ds
	mov	dx,_brkvec
	mov	bx,word ptr _brkvec+2
	mov	ds,bx
	mov	ax,2523H	;restore old cntl-break handler
	int	21H
	pop	ds
	pret
	pend	ctl_brk_restore

brk_handler proc far
	;save ds and address our data segment
	push	ds
	mov	ds,ourds
	;move to the local stack after saving dos stack
	push	ax
	mov	savess,ss
	mov	savesp,sp
	mov	ax,ds
	mov	ss,ax
	mov	sp,offset stktop
	;save registers
	push	bp
	push	bx
	push	cx
	push	dx
	push	si
	push	di
	push	es
	mov	ah,051H	;find the current psp
	int	21H
	cmp	bx,_PSP_	;is it our program segment?
	je	noabort
	;set carry flag
	mov	ax,0FFFFH	;set up to shift bit into carry
	rcr	ax,1
	jmp	short done
noabort:
	;clear carry flag
	or	ax,ax		;should clear carry
done:
	pop	es
	pop	di
	pop	si
	pop	dx
	pop	cx
	pop	bx
	pop	bp
	mov	ax,savess
	mov	ss,ax
	mov	sp,savesp
	pop	ax
	pop	ds
	jc	abortend
	iret
abortend:
	ret
brk_handler endp
	finish
	end
