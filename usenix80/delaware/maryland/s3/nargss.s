/ nargs instead of C library version.
/ Source module called nargss.s

/ This routine should only be used
/ with user I&D space separate,
/ and then only when necessary,
/ since performance is degraded.
/ Written by Robert L. Kirby.

	mfpit = 58.
	jmpi = 0167	/ jmp 0
	bri = 1		/ [br .]>>8

.globl	_nargs

_nargs:
	mov	r5,-(sp)
	mov	2(r5),r1	/ pc of caller of caller
	clr	r5
	mov	r1,r0
	sub	$4,r0
	sys	mfpit
	cmp	r0,(pc)+	/ Operand follows
	jsr	pc,*(pc)+	/ jsr pc,*$0
	bne	8f
	mov	$2,r5
8:
	mov	r1,r0
	sys	mfpit
	cmp	r0,(pc)+	/ Operand follows
	tst	(sp)+
	bne	1f
	add	$2,r5
	br	2f
1:
	cmp	r0,(pc)+	/ Operand follows
	cmp	(sp)+,(sp)+
	bne	1f
	add	$4,r5
	br	2f
1:
	cmp	r0,(pc)+	/ Operand follows
	add	(pc)+,sp	/ add $0,sp
	bne	1f
	mov	r1,r0
	add	$2,r0
	sys	mfpit
	add	r0,r5
	br	2f
1:
	cmp	r0,$jmpi
	bne	1f
	mov	r1,r0
	add	$2,r0
	sys	mfpit
	add	r0,r1
	add	$4,r1
	br	8b
1:
	swab	r0
	cmpb	r0,$bri
	bne	2f
	ash	$-7,r0
	add	r0,r1
	add	$2,r1
	br	8b
2:
	mov	r5,r0
	mov	(sp)+,r5
	asr	r0
	rts	pc
