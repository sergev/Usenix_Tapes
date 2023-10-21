**
~string	equ 	@177762
~i	equ 	@177770
~num	equ 	@4
_itoh	ldx 	#@15
	jsr 	csv
	clr a
	lda b	#@360
	ldx 	index
	and a	@33,x
	and b	@32,x
	psh a
	psh b
	lda a	#@14
	and a	#@17
	sta a	t0+1
	clr 	t0
	pul b
	pul a
	des 
	des 
	ldx 	t0
	inx 
itoh20000	dex 
	beq 	itoh20001
	asr b
	ror a
	bra 	itoh20000
itoh20001	ins 
	ins 
	ldx 	index
	sta a	@6,x
	lda a	#@17
	and a	@6,x
	add a	#@60
	sta a	@6,x
	clr a
	lda b	#@17
	and a	@33,x
	and b	@32,x
	psh a
	psh b
	lda a	#@10
	and a	#@17
	sta a	t0+1
	clr 	t0
	pul b
	pul a
	des 
	des 
	ldx 	t0
	inx 
itoh20002	dex 
	beq 	itoh20003
	asr b
	ror a
	bra 	itoh20002
itoh20003	ins 
	ins 
	add a	#@60
	ldx 	index
	sta a	@7,x
	lda a	#@360
	clr b
	and a	@33,x
	and b	@32,x
	psh a
	psh b
	lda a	#@4
	and a	#@17
	sta a	t0+1
	clr 	t0
	pul b
	pul a
	des 
	des 
	ldx 	t0
	inx 
itoh20004	dex 
	beq 	itoh20005
	asr b
	ror a
	bra 	itoh20004
itoh20005	ins 
	ins 
	add a	#@60
	ldx 	index
	sta a	@10,x
	lda a	#@17
	clr b
	and a	@33,x
	add a	#@60
	sta a	@11,x
	clr a
itoh30001	sta a	@15,x
	sta b	@14,x
	lda a	#@4
	clr b
	cmp b	@14,x
	blt 	itoh20007
	bgt 	itoh20006
	cmp a	@15,x
	bls 	itoh20007
itoh20006	lda a	#@377
	tab
	bra 	itoh20008
itoh20007	clr a
	clr b
itoh20008	tst a
	bne 	itoh30002
	jmp 	itoh3
itoh30002	lda a	#@6
	clr b
	add a	index+1
	adc b	index
	add a	@15,x
	adc b	@14,x
	sta a	c0+1
	sta b	c0
	ldx 	c0
	lda a	x
	psh a
	lda a	#@72
	tsx 
	cmp a	@0,x
	bgt 	itoh20011
	lda a	#@377
	bra 	itoh20012
itoh20011	clr a
itoh20012	ins 
	tst a
	beq 	itoh5
	lda a	#@6
	clr b
	add a	index+1
	adc b	index
	ldx 	index
	add a	@15,x
	adc b	@14,x
	sta a	x0+1
	sta b	x0
	lda a	#@6
	clr b
	add a	index+1
	adc b	index
	add a	@15,x
	adc b	@14,x
	sta a	c0+1
	sta b	c0
	ldx 	c0
	lda a	x
	add a	#@7
	ldx 	x0
	sta a	@0,x
itoh5	lda a	#@6
	clr b
	add a	index+1
	adc b	index
	ldx 	index
	add a	@15,x
	adc b	@14,x
	sta a	c0+1
	sta b	c0
	ldx 	c0
	lda a	x
	clr b
	psh a
	psh b
	jsr 	_putchar
	ins 
	ins 
	lda a	#@1
	clr b
	sta a	c0+1
	sta b	c0
	ldx 	index
	add a	@15,x
	adc b	@14,x
	jmp 	itoh30001
itoh3	jmp 	cret
