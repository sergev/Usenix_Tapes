/ disk boot program to load and transfer
/ to a unix entry

/ entry is made by jsr pc,*$0
/ so return can be rts pc

core = 28.
.. = [core*2048.]-512.

/ establish sp and check if running below
/ intended origin, if so, copy
/ program up to 'core' K words.
start:
	mov	$..,sp
	mov	sp,r1
	cmp	pc,r1
	bhis	2f
	clr	r0
	cmp	(r0),$407
	bne	1f
	mov	$20,r0
1:
	mov	(r0)+,(r1)+
	cmp	r1,$end
	blo	1b
	jmp	(sp)

/ clear core to make things clean
2:
	clr	(r0)+
	cmp	r0,sp
	blo	2b

	mov	$'@,r0	/ added: SP/HCR 28SEPT80
	jsr	pc,putc	/     "             "

/ initialize disk

	mov	$xyclear,*$xycsr	/ clear disc controller

/ at origin, read pathname,
/ spread out in array 'names', one
/ component every 14 bytes.
	mov	$names,r1
1:
	mov	r1,r2
2:
	jsr	pc,getc
	cmp	r0,$'\n
	beq	1f
	cmp	r0,$'/
	beq	3f
	movb	r0,(r2)+
	br	2b
3:
	cmp	r1,r2
	beq	2b
	add	$14.,r1
	br	1b

/ now start reading the inodes
/ starting at the root and
/ going through directories
1:
	mov	$names,r1
	mov	$2,r0
1:
	clr	bno
	jsr	pc,iget
	tst	(r1)
	beq	1f
2:
	jsr	pc,xyblk
		br start
	mov	$buf,r2
3:
	mov	r1,r3
	mov	r2,r4
	add	$16.,r2
	tst	(r4)+
	beq	5f
4:
	cmpb	(r3)+,(r4)+
	bne	5f
	cmp	r4,r2
	blo	4b
	mov	-16.(r2),r0
	add	$14.,r1
	br	1b
5:
	cmp	r2,$buf+512.
	blo	3b
	br	2b

/ read file into core until
/ a mapping error, (no disk address)
1:
	clr	r1
1:
	jsr	pc,xyblk
	br	1f
	mov	$buf,r2
2:
	mov	(r2)+,(r1)+
	cmp	r2,$buf+512.
	blo	2b
	br	1b
/ relocate core around
/ assembler header
1:
	clr	r0
	cmp	(r0),$407
	bne	2f
1:
	mov	20(r0),(r0)+
	cmp	r0,sp
	blo	1b
/ enter program and
/ restart if return
2:
	mov	$'x,r0
	jsr	pc,putc
	jsr	pc,*$0
	br	start

/ get the inode specified in r0
iget:
	add	$15.,r0
	mov	r0,r5
	ash	$-3.,r0
	bic	$!17777,r0
	mov	r0,dno
	clr	r0
	jsr	pc,rblk
	bic	$!7,r5
	ash	$6,r5
	add	$buf,r5
	mov	$inod,r4
1:
	mov	(r5)+,(r4)+
	cmp	r4,$inod+64.
	blo	1b
	rts	pc

/ read a mapped block
/ offset in file is in bno.
/ skip if success, no skip if fail
/ the algorithm only handles a single
/ indirect block. that means that
/ files longer than 10+128 blocks cannot
/ be loaded.
xyblk:
	add	$2,(sp)
	mov	bno,r0
	cmp	r0,$10.
	blt	1f
	mov	$10.,r0
1:
	mov	r0,-(sp)
	asl	r0
	add	(sp)+,r0
	add	$addr+1,r0
	movb	(r0)+,dno
	movb	(r0)+,dno+1
	movb	-3(r0),r0
	bne	1f
	tst	dno
	beq	2f
1:
	jsr	pc,rblk
	mov	bno,r0
	inc	bno
	sub	$10.,r0
	blt	1f
	ash	$2,r0
	mov	buf+2(r0),dno
	mov	buf(r0),r0
	bne	rblk
	tst	dno
	bne	rblk
2:
	sub	$2,(sp)
1:
	rts	pc

/ XYLOGICS Phoenix 211 disc controller, 300mb disc
xycsr = 164000
xytrk = 164002
xyba  = 164004
xywc  = 164006
xycyl = 164010
xyds  = 164012
xyer  = 164014
xyclear = 023	/ command to clear disc controller
cyl = 23.	/ offset to root filesystem -- site dependent
iocom = 5	/ read command + GO bit
setxmem = 30000	/ both memory extension bits for disc csr.
set1xmem = 10000 / just lower mem extension bit.

/ low order address in dno,
/ high order in r0.
rblk:
	mov	r1,-(sp)
	mov	dno,r1
	/ sector per track is 32
	/ n_heads is 5 on 80 mb disc, 19 on 300mb disc
	div	$32.*19.,r0
	add	$cyl,r0		/ cylinder no. plus filesystem offset
	mov	r0,*$xycyl
	clr	r0
	div	$32.,r0
	ash	$7.,r0
	bis	r1,r0
	mov	$xywc,r1
	mov	$-256.,(r1)
	mov	$buf,-(r1)
	mov	r0,-(r1)
	mov	$iocom,-(r1)
1:
	tstb	(r1)
	bge	1b
	mov	(sp)+,r1
	rts	pc

tks = 177560
tkb = 177562
/ read and echo a teletype character
getc:
	mov	$tks,r0
	inc	(r0)
1:
	tstb	(r0)
	bge	1b
	mov	tkb,r0
	bic	$!177,r0
	cmp	r0,$'A
	blo	1f
	cmp	r0,$'Z
	bhi	1f
	add	$'a-'A,r0
1:

tps = 177564
tpb = 177566
/ print a teletype character
putc:
	tstb	*$tps
	bge	putc
	mov	r0,*$tpb
	cmp	r0,$'\r
	bne	1f
	mov	$'\n,r0
	br	putc
1:
	rts	pc

end:
inod = ..-1024.
addr = inod+12.
buf = inod+64.
bno = buf+512.
dno = bno+2
names = dno+2
reset = 5
