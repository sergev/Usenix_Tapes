/ C library -- nap

/ error = nap(clock_ticks)

nap = 57.

.globl _nap, cerror

_nap:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),r0
	sys	nap
	bec	1f
	jmp	cerror
1:
	mov	(sp)+,r5
	rts	pc
