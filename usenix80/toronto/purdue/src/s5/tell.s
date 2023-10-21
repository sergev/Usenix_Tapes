/ C library -- tell

/ error = tell(file);

.globl	_tell, cerror
tell=40.

_tell:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),r0
	sys     tell
	bec	1f
	mov     $-1,r1
	jmp	cerror
1:
	mov	(sp)+,r5
	rts	pc
