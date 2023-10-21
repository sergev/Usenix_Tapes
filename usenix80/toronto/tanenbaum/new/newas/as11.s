/
/ copyright 1972 bell telephone laboratories inc.
/
/ PDP-11 assembler pass 0
/
/ VU: a lot of changes; warning: not all changes are marked
/ VU PATH: as2 in /ovl (not in this distributed version)
/ VU MODE: give files mode 644
/ VU MOD1: -o flag implemented
/ VU MOD2: exit with decent exit status
/ VU MOD3: handle big files (see big comment in as21.s)
/ VU MOD4: increase symbol table from 1500 to 2500

indir	= 0

	jmp	start
go:
	jsr	pc,assem
	movb	pof,r0
	sys	write; outbuf; 512.
	movb	pof,r0
	sys	close
	movb	fbfil,r0
	sys	close
	tstb	errflg
	bne	aexit
	jsr	r5,fcreat; a.tmp3
	mov	r0,r1
	mov	symend,0f
	sub	$usymtab,0f
	sys	indir; 9f
	.data
9:	sys	write; usymtab; 0:..
	.text
	mov	r1,r0
	sys	close
	sys	exec; 2f; 1f
	mov	$2f,r0
	jsr	r5,filerr; "?\n

aexit:
	sys	unlink; a.tmp1
	sys	unlink; a.tmp2
	sys	unlink; a.tmp3
	movb	errflg,r0
	sys	exit
.data
1:
	2f
	a.tmp1
	a.tmp2
	a.tmp3
a.outp:	a.out
unglob:
	3f
	0
	.text
2:
fpass2:
/#ifndef PATH
	</lib/as2\0>
/#endif
/#ifdef PATH
/	</ovl/as:2\0>
/#endif
3:
	<-g\0>
	.even

filerr:
	movb	$-1,errflg
	mov	r4,-(sp)
	mov	r0,r4
	mov	r4,0f
	clr	r0
1:
	tstb	(r4)+
	beq	1f
	inc	r0
	br	1b
1:
	mov	r0,1f
	mov	$1,r0
	sys	indir; 9f
	.data
9:	sys	write; 0:0; 1:0
	.text
	mov	r5,0f
	mov	$1,r0
	sys	indir; 9f
	.data
9:	sys	write; 0:0; 2
	.text
	tst	(r5)+
	mov	(sp)+,r4
	rts	r5

fcreat:
	mov	r4,-(sp)
	mov	(r5)+,r4
	mov	r4,0f
1:
	sys	indir; 9f
	.data
9:	sys	stat; 0:..; outbuf
	.text
	bec	2f
	mov	r4,0f
	sys	indir; 9f
	.data
9:	sys	creat; 0:..; 444
	.text
	bes	2f
	mov	(sp)+,r4
	rts	r5
2:
	incb	9.(r4)
	cmpb	9.(r4),$'z
	blos	1b
	mov	r4,r0
	jsr	r5,filerr; "?\n
	jmp	aexit
