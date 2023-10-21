/ setcsw - set software switch register
/
/ setcsw(integer);

setcsw = 56.

.globl	_setcsw, cerror

_setcsw:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),r0	/ get the argument
	sys	setcsw
	bec	1f
	jmp	cerror
1:
	mov	(sp)+,r5
	rts	pc
