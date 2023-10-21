	.data
	.text
	.globl	_out
_out:
	push	bp
	mov	bp,sp
	push	si
	push	di
	mov	dx,*4(bp)
	movb	ax,*6(bp)
	cbw
	out
	lea	sp,*-4(bp)
	pop	di
	pop	si
	pop	bp
	ret
	.data
