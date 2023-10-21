iomult	clra
	psha
	psha
	ldab	#15
	pshb
	clrb
	tsx
iom100	asr	5,x
	ror	6,x
	bcc	iom105
	adda	8,x
	adcb	7,x
iom105	asrb
	rora
	ror	1,x
	ror	2,x
	dec	0,x
	bne	iom100
	ror	6,x
	bcc	iom200
	suba	8,x
	sbcb	7,x
iom200	asrb
	ror	1,x
	ror	2,x
	ins
	pulb
	pula
	ldx	index
	rts
