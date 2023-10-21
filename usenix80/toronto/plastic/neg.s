/ Q Library -- 2's complement of the integer
/ _neg(address of number, count)
/ lcary is set if carry out
/ 

.comm _lcary,2
.globl __neg

__neg:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),r0			/ address of count
	mov	6(r5),r1			/ count
	mov	r1,-(sp)			/ save r1 for later
	mov	r0,-(sp)
	clr	_lcary
1:
	com	(r0)+				/ complement
	dec	r1
	bne	1b
	mov	(sp)+,r0			/ restore for incr
	mov	(sp)+,r1
	sec					/ set carry for add in
2:
	adc	(r0)+
	dec	r1
	bne	2b
	bcc	3f
	inc	_lcary
3:
	mov	r5,sp
	mov	(sp)+,r5
	rts	pc
