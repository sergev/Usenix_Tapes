/ read and echo character from tty.
/ perform normal cr/lf uc/lc mapping.
tks = 177560
tkb = 177562
getc:
	tst	reboot		/ use default name?
	beq	1f		/ nope, use console
	movb	*reboot,r0
	inc	reboot
	br	putc		/ echo default name
1:
	tstb	*$tks
	bge	1b
	mov	tkb,r0
	bic	$!177,r0
/	cmp	r0,$'A
/	blo	1f
/	cmp	r0,$'Z
/	bhi	1f
/	add	$'a-'A,r0
1:
	cmp	r0,$'\r
	bne	putc
	mov	$'\n,r0

/ put a character on the tty.
/ also performs delay.
tps = 177564
tpb = 177566
putc:
	cmp	r0,$'\n
	bne	1f
	mov	$'\r,r0
	jsr	pc,(r5)
	mov	$'\n,r0
1:
	tstb	tps
	bpl	1b
	mov	r0,tpb
	rts	pc

/ write a string to tty
/ jsr pc, mesg; <string\0>; .even
mesg:
	movb	*(sp),r0
	beq	1f
	jsr	pc,(r5)
	inc	(sp)
	br	mesg
1:
	add	$2,(sp)
	bic	$1,(sp)
	rts	pc

callout:
	clr	r0
	cmp	(r0),$407
	bne	2f
1:
	mov	20(r0),(r0)+
	cmp	r0,sp
	blo	1b
2:
	tst	sw
	beq	1f
	mov	sw,*$50		/ software switch register
1:
	mov	$trvect,r5
	mov	$start,-(sp)
	clr	pc

/ subroutine to save a number to be stored
/ in the software switch register

setsw:
	mov	r0,sw
	rts	pc

trvect:
	br	putc
	br	getc
	br	mesg
	br	setsw
