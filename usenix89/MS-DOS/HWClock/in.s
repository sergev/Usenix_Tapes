	.data
	.text
	.globl	_in
_in:
	push	bp
	mov	bp,sp
	push	si
	push	di
	mov	dx,*4(bp)
	in
	cbw
	lea	sp,*-4(bp)
	pop	di
	pop	si
	pop	bp
	ret
	.data
