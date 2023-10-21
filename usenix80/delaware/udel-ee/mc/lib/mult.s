mult	clra
	psha
	psha
	ldab	#15
	pshb
	clrb
	tsx
m100	asr	5,x
	ror	6,x
	bcc	m105
	adda	8,x
	adcb	7,x
m105	asrb
	rora
	ror	1,x
	ror	2,x
	dec	0,x
	bne	m100
	ror	6,x
	bcc	m200
	suba	8,x
	sbcb	7,x
m200	asrb
	ror	1,x
	ror	2,x
	ins
	pulb
	pula
	ldx	index
	rts
