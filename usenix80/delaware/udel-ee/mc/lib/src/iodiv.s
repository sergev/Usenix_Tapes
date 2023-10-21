iodiv	tsx
	tsta
	beq	iodiv4
	ldaa	2,x
	ldab	4,x
	staa	4,x
	stab	2,x
	ldaa	3,x
	ldab	5,x
	staa	5,x
	stab	3,x
iodiv4	clra
	psha
	psha	
	tsx
	cmpa	4,x
	ble	iodiv1
	com	x
	ldaa	5,x
	ldab	4,x
	comb
	nega
	tsta
	bne	*+3
	incb
	stab	4,x
	staa	5,x
iodiv1	clra
	cmpa	6,x
	ble	iodiv2
	com	x
	ldaa	7,x
	ldab	6,x
	comb
	nega
	tsta
	bne	*+3
	incb
	stab	6,x
	staa	7,x
iodiv2	ldaa	#1
	tst	4,x
	bmi	iodiv153
iodiv151	inca
	asl	5,x
	rol	4,x
	bmi	iodiv153
	cmpa	#17
	bne	iodiv151
iodiv153	staa	1,x
	ldaa	6,x
	ldab	7,x
	clr	6,x
	clr	7,x
iodiv163	subb	5,x
	sbca	4,x
	bcc	iodiv165
	addb	5,x
	adca	4,x
	clc
	bra	iodiv167
iodiv165	sec
iodiv167	rol	7,x
	rol	6,x
	lsr	4,x
	ror	5,x
	dec	1,x
	bne	iodiv163
	ldab	6,x
	ldaa	7,x
	tst	x
	beq	iodiv3
	comb
	nega
	tsta
	bne	*+3
	incb
iodiv3	ins
	ins
	ldx	index
	rts
