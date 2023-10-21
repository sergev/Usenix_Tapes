/ C library -- geteuid

/ euid = geteuid();
/
geteuid=49.

.globl	_geteuid

_geteuid:
	mov	r5,-(sp)
	mov	sp,r5
	sys	geteuid
	mov	(sp)+,r5
	rts	pc
