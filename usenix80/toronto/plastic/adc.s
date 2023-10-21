/Q library -- add with carry n words
/ _add(address of a, address of b, number of words)
/ lcary is set depending on the result
/

.globl __add
.comm _lcary,2

__add:
	mov	r5,-(sp)		/ save registers
	mov	sp,r5
	mov	r2,-(sp)
	mov	r3,-(sp)
	mov	4(r5),r0		/ r0 <- address of words
	mov	6(r5),r1		/ r1 <- address of words
	mov	010(r5),r2		/ r2 <- count
	clr	_lcary
	clr	r3
	add	(r0)+,(r1)+		/ add the first time
	rol	r3
1:
	dec	r2
	beq	3f			/ exit
	add	(r0)+,(r1)
	ror	r3			/ c <- old carry, bit 15 this carry
	adc	(r1)+			/ add in old carry
	bcs	2f
	rol	r3			/ bit 0 <- 0, c <- add carry
	rol	r3			/ bit 0 <- add carry
	br	1b
2:
	rol	r3			/ adc carry into bit 0, c <- old carry
	br	1b			/ bit 0 <- adc carry ( which is one )
3:
	mov	r3,_lcary
	mov	(sp)+,r3
	mov	(sp)+,r2
	mov	r5,sp
	mov	(sp)+,r5
	rts	pc
