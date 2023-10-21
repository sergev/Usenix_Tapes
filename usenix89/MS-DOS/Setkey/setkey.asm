;
; Entered from PC Technical Journal, May 1985, page 39
;
;name: setkey.asm (setkey.com) by Kevin M. Crenshaw
;usage: setkey [a-z] [1-4]
;
cseg	segment
assume	cs:cseg,ds:cseg
org	100h
;-----	get typematic rate: a to z, [, \, ], ^, _, or '
start:	mov	si,81h
	xor	bx,bx
letter:	lodsb
	cmp	al," "
	je	letter
	jb	send
	dec	al
	and	al,0dfh
	sub	al,"@"
	jnb	lettr1
	dec	si
	jmp	short digit
lettr1:	cmp	al,31
	ja	error2
	xchg	al,bl
;-----	get delay value: 1 to 4
digit:	lodsb
	cmp	al," "
	je	digit
	jb	send
	sub	al,"1"
	jb	error2
	cmp	al,3
	ja	error2
	mov	cl,5
	shl	al,cl
	or	bl,al
;-----	send values to keyboard
send:	mov	al,0f3h
	call	xmit
	jcxz	error1
	xchg	al,bl
	call	xmit
	jcxz	error1
	int	20h
;-----	bad input
error1:	mov	dx,offset error1$
	jmp	short error
error2:	mov	dx,offset error2$
error:	mov	ah,9
	int	21h
	int	20h
error1$	db	"Hardware error",13,10,"$"
error2$	db	"Valid parameters are A-Z, then 1-4",13,10,"$"
; xmit - send data to keyboard
; in:	al	- data to send
; out:	ax	- destroyed
;	cx	- zero if error, nonzero otherwise
xmit	proc	near
	cli
	xchg	al,ah
	xor	cx,cx
xmtwt1:	in	al,64h
	test	al,2
	loopnz	xmtwt1
	jcxz	xmtret
	xchg	al,ah
	out	60h,al
	xor	cx,cx
xmtwt2:	in	al,64h
	test	al,2
	loopnz	xmtwt2
	jcxz	xmtret
	xor	cx,cx
xmtwt3:	in	al,64h
	test	al,1
	loopz	xmtwt3
	jcxz	xmtret
	in	al,60h
	cmp	al,0fah
	je	xmtret
	xor	cx,cx
xmtret:	sti
	ret
xmit	endp
cseg	ends
	end	start
pz	xmtwt3
	jcxz	xmtret
	in	al,60h
	cmp	al,0fah
	je	xmtret
	xor	cx,cx
xmtret:	sti
	ret
;#------------end-of-code---------------------------end-of-code-------#
