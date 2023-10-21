.globl	_timelim
timelim = 53.

_timelim:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),r0
	sys	timelim
	mov	(sp)+,r5
	rts	pc
