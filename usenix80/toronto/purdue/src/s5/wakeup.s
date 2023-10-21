/ C library-- wakeup

/ error = wakeup(address)

.globl	_wakeup, cerror

wakeup = 52.
_wakeup:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(sp),r0
	sys	wakeup
	bec	1f
	jmp	cerror
1:
	clr	r0
	mov	(sp)+,r5
	rts	pc
