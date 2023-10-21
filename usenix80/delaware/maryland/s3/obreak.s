	obreak = 57.
	.globl	_obreak, _etext, cerror
.text
_obreak:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),r0
	mov	r0,r1
	sys	obreak
	bec	1f
	jmp	cerror
1:
	mov	2f,r0
	mov	r1,2f
	mov	(sp)+,r5
	rts	pc
.data
2:	_etext
