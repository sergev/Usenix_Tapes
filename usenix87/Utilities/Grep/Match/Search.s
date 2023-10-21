LL0:
	.data
	.text
	.align	1
	.globl	_Search
_Search:
	.word	L20
	matchc	8(ap),*4(ap),16(ap),*12(ap)
	beql	match
	movl	20(ap),r0
	movl	r3,8(r0)
	movl	$0,r0
	ret
match:
	movl	20(ap),r0
	movl	r3,8(r0)
	movl	$1,r0
	ret
	.set	L20,0xf00
	.data
