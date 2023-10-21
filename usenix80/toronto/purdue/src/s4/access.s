/ C library -- access

/ error = access(string);

access=33.
.globl	_access, cerror

_access:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),0f
	mov	6(r5),2f
	sys	0; 9f
	bec	1f
	jmp	cerror
1:
	clr	r0
	mov	(sp)+,r5
	rts	pc
.data
9:
	sys	access; 0:..; 2:..
