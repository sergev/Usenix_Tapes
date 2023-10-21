**
mod	tsx
	tsta
	bne	mod4
	ldaa	2,x
	ldab	4,x
	staa	4,x
	stab	2,x
	ldaa	3,x
	ldab	5,x
	staa	5,x
	stab	3,x
mod4	ldaa	5,x
	ldab	4,x
	psha
	pshb
	ldaa	3,x
	ldab	2,x
	psha
	pshb
	ldaa	#1
	jsr	moddiv
	ins
	ins
	ins
	ins
	tsx
	psha
	pshb
	ldaa	5,x
	ldab	4,x
	psha
	pshb
	jsr	modmult
	ins
	ins
	ins
	ins
	comb
	coma
	adda	#1
	adcb	#0
	tsx
	adda	3,x
	adcb	2,x
	ldx	index
	rts
modmult	clra
	psha
	psha
	ldab	#15
	pshb
	clrb
	tsx
mod100	asr	5,x
	ror	6,x
	bcc	mod105
	adda	8,x
	adcb	7,x
mod105	asrb
	rora
	ror	1,x
	ror	2,x
	dec	0,x
	bne	mod100
	ror	6,x
	bcc	mod200
	suba	8,x
	sbcb	7,x
mod200	asrb
	ror	1,x
	ror	2,x
	ins
	pulb
	pula
	ldx	index
	rts
moddiv	tsx
	tsta
	beq	moddiv4
	ldaa	2,x
	ldab	4,x
	staa	4,x
	stab	2,x
	ldaa	3,x
	ldab	5,x
	staa	5,x
	stab	3,x
moddiv4	clra
	psha
	psha	
	tsx
	cmpa	4,x
	ble	moddiv1
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
moddiv1	clra
	cmpa	6,x
	ble	moddiv2
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
moddiv2	ldaa	#1
	tst	4,x
	bmi	moddiv153
moddiv151	inca
	asl	5,x
	rol	4,x
	bmi	moddiv153
	cmpa	#17
	bne	moddiv151
moddiv153	staa	1,x
	ldaa	6,x
	ldab	7,x
	clr	6,x
	clr	7,x
moddiv163	subb	5,x
	sbca	4,x
	bcc	moddiv165
	addb	5,x
	adca	4,x
	clc
	bra	moddiv167
moddiv165	sec
moddiv167	rol	7,x
	rol	6,x
	lsr	4,x
	ror	5,x
	dec	1,x
	bne	moddiv163
	ldab	6,x
	ldaa	7,x
	tst	x
	beq	moddiv3
	comb
	nega
	tsta
	bne	*+3
	incb
moddiv3	ins
	ins
	ldx	index
	rts
