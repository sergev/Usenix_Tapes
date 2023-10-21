/ C library -- empty: RAND addition; returns 1 if pipe is empty, else 0.
/ error =  empty(file);
	empty=63.
	.globl	_empty, cerror
_empty:
	mov	r5,-(sp)
	mov	sp,r5
	mov     4(r5),r0
	sys	empty
	bec	1f
	jmp	cerror
1:
	mov	(sp)+,r5
	rts	pc
