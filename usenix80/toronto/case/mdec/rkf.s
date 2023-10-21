/ format RK03/05 disk packs

rkda = 177412

	br	start
	0; 0
start:
	jsr	pc,4(r5)
		<ready drive 2 and type y\n\0>; .even
	jsr	pc,2(r5)
	mov	r0,-(sp)
	mov	$'\n,r0
	jsr	pc,(r5)
	cmp	(sp)+,$'y
	beq	1f
	rts	pc
1:
	mov	$203.*2,r4
	mov	$40000,r3	/ drive 2
1:
	mov	$rkda+2,r0
	mov	r3,-(r0)
	mov	$buf,-(r0)
	mov	$-12.*256.,-(r0)
	mov	$6003,-(r0)
2:
	tstb	(r0)
	bge	2b
	tst	(r0)
	blt	1f
	add	$20,r3
	dec	r4
	bne	1b
	rts	pc
1:
	jsr	pc,4(r5)
		<rkf: error\n\0>; .even
	rts	pc

buf:	.=.+2
