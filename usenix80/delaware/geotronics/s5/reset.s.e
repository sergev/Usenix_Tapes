31a
ssp:	.=.+2
.
26,27c
	mov	ssp,sp
	mov	spc,(sp)
	rts	pc
.
24a
	jsr	r5,csv
	mov	4(r5),r0
1:
	cmp	(r5),sr5
	beq	1f
	mov	(r5),r5
	bne	1b
/ panic -- r2-r4 lost
	br	2f
1:
	mov	-(r5),r4
	mov	-(r5),r3
	mov	-(r5),r2
2:
.
21,22c
	mov	sp,ssp
	mov	(sp),spc
	clr	r0
	rts	pc
.
19d
8a
/ 'val' is returned to the caller of setexit.
/ setexit itself returns 0.
.
3c
/	modified 05-Jun-1980 by D A Gwyn:
/	1) new version.

/	reset( val )
.
1c
/ reset.s - C library routines reset, setexit
.
w
q
