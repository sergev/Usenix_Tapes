/Q library -- n word arithmetic bit shift left
/ __asl(address of first word, number of words)
/ lcary is set depending on the result
/

.globl __asl
.comm _lcary,2

__asl:
	mov	r5,-(sp)		/ save registers
	mov	sp,r5
	mov	4(r5),r0		/ r0 <- address of words
	mov	6(r5),r1		/ r1 <- count
	clr	_lcary
	asl	(r0)+			/ shift in zero
1:
	dec	r1
	beq	2f			/ exit
	rol	(r0)+
	br	1b
2:
	bcc	3f
	inc	_lcary
3:
	mov	r5,sp
	mov	(sp)+,r5
	rts	pc
