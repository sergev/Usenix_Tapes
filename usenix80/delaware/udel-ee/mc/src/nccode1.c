/*
	This is the 6800 code table.  References to it are made be absolute
	index within it, as indicated by the numerical comments.  The format
	is standard printf.
 */

char *code[] {
"	stx	t0\n\
	psha\n\
	pshb\n\
	ldaa	index+1\n\
	ldab	index\n\
	adda	#@%o\n\
	adcb	#@%o\n\
	staa	c0+1\n\
	stab	c0\n\
	pulb\n\
	pula\n\
	ldx	c0\n",
"	ldaa	x\n",
"	ldx	t0\n",
"	clrb\n",
"	ldaa	@%o,x\n",
"	ldaa	_%.8s+@%o\n",
"	ldaa	L%d\n",
"	ldab	@%o,x\n",
"	ldaa	1,x\n",
"	ldab	x\n",		/* 10 */
"	ldaa	_%.8s+@%o\n",
"	ldab	_%.8s+@%o\n",
"	ldaa	L%d+1\n",
"	ldab	L%d\n",
"	ldaa	#@%o\n",
"	ldab	#@%o\n",
"	staa	x\n",
"	staa	@%o,x\n",
"	staa	_%.8s+@%o\n",
"	staa	L%d\n",		/* 20 */
"	staa	1,x\n",
"	stab	x\n",
"	stab	@%o,x\n",
"	staa	_%.8s+@%o\n",
"	stab	_%.8s+@%o\n",
"	staa	L%d+1\n",
"	stab	L%d\n",
"	stx	t0\n",
"	tsx\n",
"	ldx	x\n",		/* 30 */
"	coma\n",
"	comb\n",
"	tsta\n",
"	bne	*+%d\n",
"	tstb\n",
"	bra	*+%d\n",
"	clra\n",
"	nega\n",
"	adcb	#@%o\n",
"	adda	x\n",		/* 40 */
"	adda	@%o,x\n",
"	adda	_%.8s+@%o\n",
"	adda	L%d\n",
"	adda	1,x\n",
"	adcb	x\n",
"	adcb	@%o,x\n",
"	adda	_%.8s+@%o\n",
"	adcb	_%.8s+@%o\n",
"	adda	L%d+1\n",
"	adcb	L%d\n",		/* 50 */
"	anda	x\n",
"	anda	@%o,x\n",
"	anda	_%.8s+@%o\n",
"	anda	L%d\n",
"	anda	1,x\n",
"	andb	x\n",
"	andb	@%o,x\n",
"	anda	_%.8s+@%o\n",
"	andb	_%.8s+@%o\n",
"	anda	L%d+1\n",	/* 60 */
"	andb	L%d\n",
"	oraa	x\n",
"	oraa	@%o,x\n",
"	oraa	_%.8s+@%o\n",
"	oraa	L%d\n",
"	oraa	1,x\n",
"	orab	x\n",
"	orab	@%o,x\n",
"	oraa	_%.8s+@%o\n",
"	orab	_%.8s+@%o\n",		/* 70 */
"	oraa	L%d+1\n",
"	orab	L%d\n",
"	eora	x\n",
"	eora	@%o,x\n",
"	eora	_%.8s+@%o\n",
"	eora	L%d\n",
"	eora	1,x\n",
"	eorb	x\n",
"	eorb	@%o,x\n",
"	eora	_%.8s+@%o\n",	/* 80 */
"	eorb	_%.8s+@%o\n",
"	eora	L%d+1\n",
"	eorb	L%d\n",
"	clrb\n",	/*  sign extend */
"	bne	L%d\n",
"	beq	L%d\n",
"L%d	equ	*\n",
"	bra	L%d\n",
"	tab\n",
"	staa	c0+1\n",	/* 90 */
"	stab	c0\n",
"	ldx	c0\n",
"	jmp	L%d\n",
"	pula\n",
"	pulb\n",
"	anda	#@%o\n",
"	clr	c0\n",
"	asla\n",
"	rolb\n",
"	asra\n",		/* 100 */
"	asrb\n",
"	rora\n",
"	dex\n",
"	adda	#@%o\n",
"	oraa	#@%o\n",
"	orab	#@%o\n",
"	eora	#@%o\n",
"	eorb	#@%o\n",
"	anda	#@%o\n",
"	andb	#@%o\n",	/* 110 */
"	cmpa	@%o,x\n",
"	cmpa	_%.8s+@%o\n",
"	cmpa	L%d\n",
"	cmpb	@%o,x\n",
"	cmpa	1,x\n",
"	cmpa	_%.8s+@%o\n",
"	cmpb	_%.8s+@%o\n",
"	cmpa	L%d+1\n",
"	cmpb	L%d\n",
"	cmpa	#@%o\n",	/* 120 */
"	cmpb	#@%o\n",
"*	fudge%d\n",
"	blt	L%d\n",
"	bgt	L%d\n",
"	bcs	L%d\n",
"	bhi	L%d\n",
"	ble	L%d\n",
"	bls	L%d\n",
"	bcc	L%d\n",
"	bge	L%d\n",		/* 130 */
"	suba	c0+1\n",
"	sbcb	c0\n",
"	adda	c0+1\n",
"	adcb	c0\n",
"	adda	t0+1\n",
"	adcb	t0\n",
"L%d	fdb	_%.8s+@%o\n",
"L%d	fdb	L%d\n",
"	psha\n",
"	pshb\n",		/* 140 */
"	jsr	_%.8s\n",
"	staa	t0+1\n",
"	stab	t0\n",
"	ins\n",
"	ldx	index\n",
"	clr	t0\n",
"	adda	index+1\n",
"	adcb	index\n",
"	des\n",
"	incb\n",		/* 150 */
"	jsr	%s\n",
"	staa	x0+1\n",
"	stab	x0\n",
"	ldx	x0\n",
"	ldaa	x0+1\n",
"	ldab	x0\n",
"	ldaa	x0\n",
"	inx\n",
"	fdb	L%d\n",
"	fdb	_%.8s+@%o\n",		/* 160 */
"	fdb	%d\n"
};
