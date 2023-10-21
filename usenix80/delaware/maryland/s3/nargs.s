/ C library -- nargs

/ WARNING: this routine does not work
/ with user I&D space separate.
/ Use the version calling mfpit instead
/ with user I&D space separate.
/ RLK altered to use immediate operands.

	jmpi = 0167	/ jmp 0
	bri = 1		/ [br .]>>8

.globl	_nargs

_nargs:
	mov	2(r5),r1	/ pc of caller of caller
	clr	r0
	cmp	-4(r1),(pc)+	/ Operand follows
	jsr	pc,*(pc)+	/ jsr pc,*$0
	bne	8f
	mov	$2,r0
8:
	cmp	(r1),(pc)+	/ Operand follows
	tst	(sp)+
	bne	1f
	add	$2,r0
	br	2f
1:
	cmp	(r1),(pc)+	/ Operand follows
	cmp	(sp)+,(sp)+
	bne	1f
	add	$4,r0
	br	2f
1:
	cmp	(r1),(pc)+	/ Operand follows
	add	(pc)+,sp	/ add $0,sp
	bne	1f
	add	2(r1),r0
	br	2f
1:
	cmp	(r1),$jmpi
	bne	1f
	add	2(r1),r1
	add	$4,r1
	br	8b
1:
	cmpb	1(r1),$bri
	bne	2f
	mov	r0,-(sp)
	mov	(r1),r0
	swab	r0
	ash	$-7,r0
	add	r0,r1
	add	$2,r1
	mov	(sp)+,r0
	br	8b
2:
	asr	r0
	rts	pc
