/ C library -- setwin : set catchable trap window

/ setwin(lo,hi);
/       char *lo, *hi; /* send SIGSYS when trap instr pc is >=lo and <=hi

setwin = 62.

.globl  _setwin

_setwin:
	mov     r5,-(sp)
	mov     sp,r5
	mov     4(r5),r0
	mov     6(r5),r1
	sys     setwin
	mov     (sp)+,r5
	rts     pc
