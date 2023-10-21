	.globl	.oVncs
	.set	.oVncs,0
	.globl	_setOOPSerror
	.globl	_.setOOPSerror
	.globl	FPaGET0
	.globl	FPaPUT0
	.globl	fplm2
	.globl	fpstm2
	.globl	_OOPS_BADTRACETBL
	.globl	_scheduler
	.text
	.align	1
L000:
_._Object_className:
	stm   	r11,-56(r1)
	mr    	r14,r0
	mr    	r13,r1
	cal   	r1,-56(r1)
	mr    	r12,r2
	ls    	r2,0(r12)
	ls    	r3,44(r2)
	mr    	r2,r12
	ls    	r15,0(r3)
	balrx 	r15,r15
	cal   	r0,0(r3)
	mr    	r11,r2
	ls    	r2,12(r11)
	mr    	r1,r13
	lm    	r11,-56(r1)
	br    	r15
	.long	0xDF07DFB8	# First gpr=r11
	.short	0x1D00	# npars=1, off=0
	.data	1
__Object_className:
	.long	L000
	.align	2
	.text
	.align	1
L02E:
	.globl	_._Process_create
_._Process_create:
	stm   	r6,-76(r1)
	st    	r4,-8(r1)
	st    	r5,-4(r1)
	mr    	r14,r0
	cal   	r12,-108(r1)
	bali  	r15,fpstm2
	ai    	r13,r1,-136
	cal   	r1,-140(r1)
	st    	r2,120(r13)
	st    	r3,124(r13)
	mr    	r9,r15
L05C:
	lhas  	r2,0(r9)
	inc   	r9,2
	nilz  	r2,r2,-256
	cal16 	r3,-8448(r0)
	c     	r2,r3
	jne   	L05C
	lhas  	r2,0(r9)
	nilz  	r2,r2,-256
	c     	r2,r3
	jne   	L05C
	dec   	r9,2
	mr    	r8,r9
	lcs   	r2,1(r8)
	cis   	r2,7
	je    	L0AE
	l     	r6,120(r13)
	mr    	r2,r6
	balix 	r15,_._Object_className	# _Object_className
	l     	r0,8(r14)
	sts   	r6,0(r1)
	mr    	r6,r2
	load	r2,_OOPS_BADTRACETBL
	cal   	r3,-4(r0)
	mr    	r4,r8
	mr    	r5,r6
	balix 	r15,_.setOOPSerror
	l     	r0,12(r14)
	b     	L00214
L0AE:
	lcs   	r2,4(r8)
	nilz  	r2,r2,15
	sfi   	r2,r2,15
	sfi   	r2,r2,-5
	sli   	r2,2
	cal   	r3,124(r13)
	a     	r2,r3
	cal   	r7,-8(r2)
	lcs   	r2,4(r8)
	sri   	r2,4
	nilz  	r2,r2,15
	sis   	r2,5
	ls    	r4,4(r8)
	sri   	r4,8
	nilz  	r4,r4,63
	a     	r2,r4
	sli   	r2,2
	ais   	r2,4
	ls    	r4,0(r7)
	a     	r2,r4
	sts   	r2,24(r13)
	mr    	r4,r1
	sf    	r4,r2
	sari  	r4,2
	sts   	r4,20(r13)
	ls    	r2,0(r3)
	sts   	r2,16(r13)
L0F2:
	ls    	r2,20(r13)
	mr    	r3,r2
	sis   	r2,1
	cis   	r3,0
	bex   	L00114
	sts   	r2,20(r13)
	ls    	r2,16(r13)
	dec   	r2,4
	sts   	r2,16(r13)
	ls    	r3,24(r13)
	dec   	r3,4
	sts   	r3,24(r13)
	ls    	r3,0(r3)
	bx    	L0F2
	sts   	r3,0(r2)
L00114:
	ls    	r2,16(r13)
	ls    	r3,24(r13)
	s     	r2,r3
	sari  	r2,2
	sli   	r2,2
	cas   	r3,r2,r7
	ls    	r4,0(r3)
	a     	r4,r2
	a     	r2,r13
	sts   	r4,0(r3)
	l	r3,120(r13)
	sts	r2,8(r3)
L00214:
	ai    	r1,r13,136
	cal   	r12,-108(r1)
	bali  	r15,fplm2
	lm    	r6,-76(r1)
	br    	r15
	.long	0xDF07DF6C	# First gpr=r6
	.short	0x2D20	# npars=2
	.short	0x2200	# off=34
	.data	1
	.globl	__Process_create
__Process_create:
	.long	L02E
	.long	0
	.long	__Object_className
	.long	_setOOPSerror
	.align	2
	.text
	.align	1
L0022E:
	.globl	_._Process_exchj
_._Process_exchj:
	stm   	r6,-76(r1)
	mr    	r14,r0
	cal   	r12,-108(r1)
	bali  	r15,fpstm2
	ai    	r13,r1,-136
	load	r3,_scheduler+8
	sts   	r13,8(r3)
	ls    	r13,8(r2)
	ai    	r1,r13,136
	cal   	r12,-108(r1)
	bali  	r15,fplm2
	lm    	r6,-76(r1)
	br    	r15
	.long	0xDF07DF6C	# First gpr=r6
	.short	0x1D20	# npars=1
	.short	0x2200	# off=34
	.data	1
	.globl	__Process_exchj
__Process_exchj:
	.long	L0022E
	.align	2
	.data
	.space	4
