	readnl = 59.
	.globl	_readnl, cerror
.text
_readnl:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),r0
	mov	6(r5),0f
	mov	8(r5),0f+2
	sys	0; 9f
	bec	1f
	jmp	cerror
1:
	mov	(sp)+,r5
	rts	pc
.data
9:	sys	readnl; 0: ..; ..;
