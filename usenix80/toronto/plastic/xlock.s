/ PO library -- xlock

/ error =  xlock(file);

.globl	_xlock, cerror

xlock = 62.
_xlock:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),r0
	sys	xlock
	bec	1f
	jmp	cerror
1:
	clr	r0
	mov	(sp)+,r5
	rts	pc
