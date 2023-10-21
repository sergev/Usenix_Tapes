/ csw - read an octal number from the
/ console and save it for the software
/ switch register.  works very closely
/ with fsboot.  this really should be
/ in fsboot, but fsboot is already too
/ big.  this is a kludge.
/
/ Bill Shannon   3/25/79

0:
	jsr	pc,4(r5); <\r\nSW = \0>; .even
	clr	r1
1:
	jsr	pc,2(r5)
	cmp	r0,$'\n
	beq	2f
	cmp	r0,$'0
	blt	0b
	cmp	r0,$'7
	bgt	0b
	sub	$'0,r0
	ash	$3,r1
	add	r0,r1
	br	1b
2:
	mov	r1,r0
	jsr	pc,6(r5)
	rts	pc
