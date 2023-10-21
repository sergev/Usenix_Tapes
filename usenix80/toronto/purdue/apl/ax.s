/
/ the following code was stolen from RAND corp.
/

.globl	_setexit,_reset
.globl	csv,_envsave,_envrest

	.text
_setexit:
	mov	(sp)+,spc
	mov	sp,ssp
	mov	r1,sr1
	mov	r2,sr2
	mov	r3,sr3
	mov	r4,sr4
	mov	r5,sr5
	jmp	*spc

_reset:
	mov	ssp,sp
	mov	sr1,r1
	mov	sr2,r2
	mov	sr3,r3
	mov	sr4,r4
	mov	sr5,r5
	jmp	*spc


_envsave:
	jsr	r5,csv
	mov	r5,*4(r5)
	mov	(r5),r5
	mov	$1,r0		/ return 1
	jmp	*10.(sp)

_envrest:
	mov	*2(sp),r1
	mov	(r1),r5
	mov	-(r1),r4
	mov	-(r1),r3
	mov	-(r1),r2
	tst	-(r1)
	mov	r1,sp
	clr	r0		/ return 0
	jmp	*10.(sp)


	.data
sr1:	0
sr2:	0
sr3:	0
sr4:	0
sr5:	0
ssp:	0
spc:	0
