/ clrtty - clear controlling tty  (super-user only)
/ 12/11/76  G. Goble
/ C support routine

clrtty=50.
.globl	_clrtty, cerror

_clrtty:
	mov	r5,-(sp)
	mov	sp,r5
	sys	clrtty
	bec	1f
	jmp	cerror

1:	clr	r0
	mov	(sp)+,r5
	rts	pc
