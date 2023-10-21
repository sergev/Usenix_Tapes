.globl	dump,_dump
dump:
_dump:
	bic	$1,SSR0

/ save regs r0,r1,r2,r3,r4,r5,r6,ka6
/ starting at abs location 4

	mov	r0,4
	mov	$6,r0
	mov	r1,(r0)+
	mov	r2,(r0)+
	mov	r3,(r0)+
	mov	r4,(r0)+
	mov	r5,(r0)+
	mov	sp,(r0)+
	mov	*_ka6,(r0)+

/ dump all of core (ie to first disk error)
/ onto rk2 (RK05j) starting at block zero.

.if	DUMP

	halt
	mov	$RKCS,r0
	mov	$2,(r0)		/ set write opcode
1:
	tstb	(r0)
	bge	1b
	mov	$RKDA,r0
	mov	$40000,(r0)	/ drive 2, disk address
	clr	-(r0)		/ bus address
	clr	-(r0)		/ word count
1:
	mov	$-256.,(r0)	/ word count
	inc	-(r0)		/ go
2:
	tstb	(r0)
	bge	2b
	tst	(r0)+
	bge	1b

/ end of file and loop


.endif
	reset
	halt

RKDA	= 177412
RKCS	= 177404
