/Q library -- n word arithmetic bit shift right
/ __asr(address of first word, number of words)
/ lcary is set to the value shifted out
/

.globl __asr
.comm _lcary,2

__asr:
	mov	r5,-(sp)		/ save registers
	mov	sp,r5
	mov	r2,-(sp)
	mov	4(r5),r0		/ r0 <- address of words
	mov	6(r5),r1		/ r1 <- count
	mov	r1,r2			/ save true count
	asl	r2			/ (offset to last word) * 2
	add	r2,r0
	clr	_lcary
	asr	-(r0)			/ first word
1:
	dec	r1
	beq	2f			/ exit
	ror	-(r0)
	br	1b
2:
	bcc	3f
	inc	_lcary
3:
	mov	(sp)+,r2
	mov	r5,sp
	mov	(sp)+,r5
	rts	pc
