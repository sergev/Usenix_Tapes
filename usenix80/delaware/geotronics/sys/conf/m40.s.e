714a
/ initialize systems segments

.
713c
/ Set loc. 0 to trap to system, in case of
/ hardware glitch
	mov	$trap,0
	mov	$340+15.,2
.
576a
	bic	$140,PS
	rts	pc

_spl5:
	bis	$340,PS
.
575d
533a
_waitloc:
.
529c
.globl	_idle, _waitloc
.
216c
/ simulate the SSR1 register missing on 11/34
.
147c
	bic	$40,PS		/ spl 6
.
108c
	bic	$40,PS		/ spl 6
.
65a
.if .fpp
	mov	$_u+4,r1
	bit	$20,(r1)	/ flags "from user mode"
	beq	1f
	stfps	(r1)+
	setd			/ force 64-bit store
	movf	fr0,(r1)+
	movf	fr4,fr0
	movf	fr0,(r1)+
	movf	fr5,fr0
	movf	fr0,(r1)+
	movf	fr1,(r1)+
	movf	fr2,(r1)+
	movf	fr3,(r1)+
1:
.endif
.
50a
.if .fpp
	mov	$_u+4,r1
	bit	$20,(r1)	/ flags "from user mode"
	bne	2f
	mov	(r1)+,r0
	setd			/ force 64-bit load
	movf	(r1)+,fr0
	movf	(r1)+,fr1
	movf	fr1,fr4
	movf	(r1)+,fr1
	movf	fr1,fr5
	movf	(r1)+,fr1
	movf	(r1)+,fr2
	movf	(r1)+,fr3
	ldfps	r0
2:
.endif
.
47a
	jsr	pc,_savfp
.
41a
.if .fpp
	mov	$20,_u+4	/ FP-11 maint mode bit not used
.endif
.
6a
ldfps	= 170100^tst
stfps	= 170200^tst
.
3a
.fpp = 1			/ enable FP-11 context switching

.
1,2c
/ m40.s - machine language assist
/	modified 04-Jun-1980 by D A Gwyn:
/	1) installed FP11 support;
/	2) installed vector to trap at loc. 0;
/	3) fixed FP11 load/store precision bug.
.
w
q
