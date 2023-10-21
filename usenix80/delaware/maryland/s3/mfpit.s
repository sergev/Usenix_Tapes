	mfpit = 58.
	.globl	_mfpit
.text
_mfpit:
	mov	2(sp),r0
	sys	mfpit
	rts	pc
