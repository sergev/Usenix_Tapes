div	tsx
	tsta
	beq	div4
	ldaa	2,x
	ldab	4,x
	staa	4,x
	stab	2,x
	ldaa	3,x
	ldab	5,x
	staa	5,x
	stab	3,x
div4	clra
	psha
	psha	
	tsx
	cmpa	4,x
	ble	div1
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
div1	clra
	cmpa	6,x
	ble	div2
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
div2	ldaa	#1
	tst	4,x
	bmi	div153
div151	inca
	asl	5,x
	rol	4,x
	bmi	div153
	cmpa	#17
	bne	div151
div153	staa	1,x
	ldaa	6,x
	ldab	7,x
	clr	6,x
	clr	7,x
div163	subb	5,x
	sbca	4,x
	bcc	div165
	addb	5,x
	adca	4,x
	clc
	bra	div167
div165	sec
div167	rol	7,x
	rol	6,x
	lsr	4,x
	ror	5,x
	dec	1,x
	bne	div163
	ldab	6,x
	ldaa	7,x
	tst	x
	beq	div3
	comb
	nega
	tsta
	bne	*+3
	incb
div3	ins
	ins
	ldx	index
	rts
