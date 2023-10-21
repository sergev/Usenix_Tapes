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
**
~c	equ 	@177764
~p	equ 	@177770
~con	equ 	@177766
_getchar	ldx 	#@13
	jsr 	csv
	lda a	#@10
	lda b	#@200
	ldx 	index
	sta a	@11,x
	sta b	@10,x
	lda a	#@11
	sta a	@13,x
	sta b	@12,x
io12	ldx 	index
	lda a	@11,x
	lda b	@10,x
	sta a	c0+1
	sta b	c0
	ldx 	c0
	lda a	x
	clr b
	and a	#@1
	psh a
	psh b
	clr a
	tsx 
	cmp b	@0,x
	bne 	io120001
	cmp a	1,x
	bne 	io120001
	lda a	#@377
	bra 	io120002
io120001	clr a
io120002	ins 
	ins 
	tst a
	bne 	io12
	ldx 	index
	lda a	@13,x
	lda b	@12,x
	sta a	c0+1
	sta b	c0
	ldx 	c0
	lda a	x
	and a	#@177
	ldx 	index
	sta a	@6,x
	lda a	#@15
	cmp a	@6,x
	bne 	io14
	lda a	#@12
	sta a	@6,x
io14	lda a	@6,x
	clr b
	psh a
	psh b
	jsr 	_putchar
	ins 
	ins 
	lda a	@6,x
	clr b
	jmp 	cret
**
~c	equ 	@4
~p	equ 	@177770
~con	equ 	@177766
_putchar	ldx 	#@11
	jsr 	csv
	lda a	#@10
	lda b	#@200
	ldx 	index
	sta a	@7,x
	sta b	@6,x
	lda a	#@11
	sta a	@11,x
	sta b	@10,x
io22	ldx 	index
	lda a	@7,x
	lda b	@6,x
	sta a	c0+1
	sta b	c0
	ldx 	c0
	lda a	x
	clr b
	and a	#@2
	psh a
	psh b
	clr a
	tsx 
	cmp b	@0,x
	bne 	io220001
	cmp a	1,x
	bne 	io220001
	lda a	#@377
	bra 	io220002
io220001	clr a
io220002	ins 
	ins 
	tst a
	bne 	io22
	ldx 	index
	lda a	@11,x
	lda b	@10,x
	sta a	x0+1
	sta b	x0
	psh a
	psh b
	lda a	@27,x
	ldx 	x0
	sta a	@0,x
	ins 
	ins 
	lda a	#@12
	ldx 	index
	cmp a	@27,x
	bne 	io220005
	lda a	#@377
	tab
	bra 	io220006
io220005	clr a
	clr b
io220006	tst a
	beq 	io21
	lda a	#@15
	clr b
	psh a
	psh b
	jsr 	_putchar
	ins 
	ins 
io21	jmp 	cret
io315	fcb 	@55,@63,@62,@67,@66,@70
	fcb 	@0
io320	fcb 	@156,@165,@154,@154,@0
**
~argp	equ 	@177764
~argv	equ 	@6
~dnum	equ 	@177766
~format	equ 	@4
~strp	equ 	@177770
~delay	equ 	@177762
io320020	fdb	io315
io320026	fdb	io320
io320029	fdb 	io310002-io310001
io310001	fdb 	@143
	fdb 	@144
	fdb 	@150
	fdb 	@157
	fdb 	@163
io310002	rmb 	2
	fdb	io311
	fdb	io312
	fdb	io317
	fdb	io316
	fdb	io318
	fdb	io330022
_printf	ldx 	#@15
	jsr 	csv
	clr a
	clr b
	ldx 	index
io330024	sta a	@7,x
	sta b	@6,x
	lda a	#@160
	lda b	#@27
	cmp b	@6,x
	blt 	io33
	bgt 	io330025
	cmp a	@7,x
	bls 	io33
io330025	lda a	#@1
	clr b
	sta a	c0+1
	sta b	c0
	add a	@7,x
	adc b	@6,x
	bra 	io330024
io33	lda a	#@34
	clr b
	add a	index+1
	adc b	index
	sta a	@11,x
	sta b	@10,x
io35	lda a	@33,x
	lda b	@32,x
	sta a	c0+1
	sta b	c0
	ldx 	c0
	lda a	x
	clr b
	psh a
	psh b
	clr a
	tsx 
	cmp b	@0,x
	bne 	io320004
	cmp a	1,x
	beq 	io320005
io320004	lda a	#@377
	tab
	bra 	io320006
io320005	clr a
	clr b
io320006	ins 
	ins 
	tst a
	bne 	io330028
	jmp 	io31
io330028	ldx 	index
	lda a	@33,x
	lda b	@32,x
	sta a	c0+1
	sta b	c0
	ldx 	c0
	lda a	x
	clr b
	psh a
	psh b
	lda a	#@45
	tsx 
	cmp b	@0,x
	bne 	io320008
	cmp a	1,x
	beq 	io320009
io320008	lda a	#@377
	bra 	io320010
io320009	clr a
io320010	ins 
	ins 
	tst a
	beq 	io37
io330022	ldx 	index
	lda a	@33,x
	lda b	@32,x
	sta a	c0+1
	sta b	c0
	ldx 	c0
	lda a	x
	clr b
io330006	psh a
	psh b
	jsr 	_putchar
io330008	ins 
	ins 
	jmp 	io38
io37	lda a	#@1
	clr b
	sta a	c0+1
	sta b	c0
	ldx 	index
	add a	@33,x
	adc b	@32,x
	sta a	@33,x
	sta b	@32,x
	sta a	c0+1
	sta b	c0
	ldx 	c0
	lda a	x
	clr b
	jmp 	io310
io311	lda a	#@2
	clr b
	sta a	c0+1
	sta b	c0
	ldx 	index
	add a	@11,x
	adc b	@10,x
	sta a	@11,x
	sta b	@10,x
	sub a	c0+1
	sbc b	c0
	sta a	c0+1
	sta b	c0
	ldx 	c0
	lda a	1,x
	lda b	x
	bra 	io330006
io312	lda a	#@2
	clr b
	sta a	c0+1
	sta b	c0
	ldx 	index
	add a	@11,x
	adc b	@10,x
	sta a	@11,x
	sta b	@10,x
	sub a	c0+1
	sbc b	c0
	sta a	c0+1
	sta b	c0
	ldx 	c0
	lda a	1,x
	lda b	x
	ldx 	index
	sta a	@13,x
	sta b	@12,x
	clr a
	clr b
	cmp b	@12,x
	blt 	io313
	bgt 	io330026
	cmp a	@13,x
	bls 	io313
io330026	lda a	@13,x
	lda b	@12,x
	com b
	com a
	add a	#@1
	adc b	#@0
	sta a	@13,x
	sta b	@12,x
	clr a
	clr b
	cmp b	@12,x
	blt 	io314
	bgt 	io330027
	cmp a	@13,x
	bls 	io314
io330027	lda a	io320020+1
	lda b	io320020
	psh a
	psh b
	jsr 	_printf
	bra 	io330029
io314	lda a	#@55
	clr b
	psh a
	psh b
	jsr 	_putchar
	ins 
	ins 
io313	lda a	@13,x
	lda b	@12,x
	psh a
	psh b
	jsr 	_printd
io330029	bra 	io330030
io316	lda a	#@2
	clr b
	sta a	c0+1
	sta b	c0
	ldx 	index
	add a	@11,x
	adc b	@10,x
	sta a	@11,x
	sta b	@10,x
	sub a	c0+1
	sbc b	c0
	sta a	c0+1
	sta b	c0
	ldx 	c0
	lda a	1,x
	lda b	x
	psh a
	psh b
	jsr 	_printo
io330030	bra 	io330031
io317	lda a	#@2
	clr b
	sta a	c0+1
	sta b	c0
	ldx 	index
	add a	@11,x
	adc b	@10,x
	sta a	@11,x
	sta b	@10,x
	sub a	c0+1
	sbc b	c0
	sta a	c0+1
	sta b	c0
	ldx 	c0
	lda a	1,x
	lda b	x
	psh a
	psh b
	jsr 	_printh
io330031	jmp 	io330008
io318	lda a	#@2
	clr b
	sta a	c0+1
	sta b	c0
	ldx 	index
	add a	@11,x
	adc b	@10,x
	sta a	@11,x
	sta b	@10,x
	sub a	c0+1
	sbc b	c0
	sta a	c0+1
	sta b	c0
	ldx 	c0
	lda a	1,x
	lda b	x
	ldx 	index
	sta a	@15,x
	sta b	@14,x
	sta a	c0+1
	sta b	c0
	ldx 	c0
	lda a	x
	clr b
	psh a
	psh b
	clr a
	tsx 
	cmp b	@0,x
	bne 	io320023
	cmp a	1,x
	bne 	io320023
	lda a	#@377
	bra 	io320024
io320023	clr a
io320024	ins 
	ins 
	tst a
	beq 	io321
	lda a	io320026+1
	lda b	io320026
	psh a
	psh b
	jsr 	_printf
io330001	ins 
	ins 
io321	ldx 	index
	lda a	@15,x
	lda b	@14,x
	sta a	c0+1
	sta b	c0
	ldx 	c0
	lda a	x
	tst a
	beq 	io38
	lda a	#@1
	clr b
	sta a	c0+1
	sta b	c0
	ldx 	index
	add a	@15,x
	adc b	@14,x
	sta a	@15,x
	sta b	@14,x
	sub a	c0+1
	sbc b	c0
	sta a	c0+1
	sta b	c0
	ldx 	c0
	lda a	x
	clr b
	psh a
	psh b
	jsr 	_putchar
	bra 	io330001
io310	sta b	io310002
	sta a	io310002+1
	ldx 	#io310001-2
io310003	inx 
	inx 
	cmp a	1,x
	bne 	io310003
	cmp b	x
	bne 	io310003
	inx 
	inx 
	stx 	c0
	lda a	c0+1
	lda b	c0
	add a	io320029+1
	adc b	io320029
	sta a	c0+1
	sta b	c0
	ldx 	c0
	ldx 	x
	jmp 	x
io38	lda a	#@1
	clr b
	sta a	c0+1
	sta b	c0
	ldx 	index
	add a	@33,x
	adc b	@32,x
	sta a	@33,x
	sta b	@32,x
	jmp 	io35
io31	jmp 	cret
**
~newnum	equ 	@177770
~num	equ 	@4
_printd	ldx 	#@7
	jsr 	csv
	lda a	#@12
	clr b
	psh a
	psh b
	ldx 	index
	lda a	@25,x
	lda b	@24,x
	psh a
	psh b
	lda a	#@1
	jsr 	iodiv
	ins 
	ins 
	ins 
	ins 
	sta a	@7,x
	sta b	@6,x
	psh a
	psh b
	clr a
	clr b
	tsx 
	cmp b	@0,x
	bne 	io320030
	cmp a	1,x
	beq 	io320031
io320030	lda a	#@377
	bra 	io320032
io320031	clr a
io320032	ins 
	ins 
	tst a
	beq 	io325
	ldx 	index
	lda a	@7,x
	lda b	@6,x
	psh a
	psh b
	jsr 	_printd
	ins 
	ins 
io325	lda a	#@12
	clr b
	psh a
	psh b
	ldx 	index
	lda a	@7,x
	lda b	@6,x
	psh a
	psh b
	jsr 	iomult
	ins 
	ins 
	ins 
	ins 
	com b
	com a
	add a	#@1
	adc b	#@0
	add a	@25,x
	adc b	@24,x
	add a	#@60
	adc b	#@0
	psh a
	psh b
	jsr 	_putchar
	ins 
	ins 
	jmp 	cret
**
~num	equ 	@4
_printo	ldx 	#@5
	jsr 	csv
	clr a
	clr b
	ldx 	index
	cmp b	@22,x
	bne 	io330032
	cmp a	@23,x
	beq 	io327
io330032	lda a	#@3
	and a	#@17
	sta a	t0+1
	clr 	t0
	lda a	@23,x
	lda b	@22,x
	ldx 	t0
	inx 
io320038	dex 
	beq 	io320039
	asr b
	ror a
	bra 	io320038
io320039	and a	#@377
	and b	#@37
	psh a
	psh b
	jsr 	_printo
	ins 
	ins 
io327	lda a	#@7
	clr b
	ldx 	index
	and a	@23,x
	and b	@22,x
	add a	#@60
	adc b	#@0
	psh a
	psh b
	jsr 	_putchar
	ins 
	ins 
	jmp 	cret
**
~num	equ 	@4
_printh	ldx 	#@5
	jsr 	csv
	clr a
	clr b
	ldx 	index
	cmp b	@22,x
	bne 	io330041
	cmp a	@23,x
	beq 	io329
io330041	lda a	#@4
	and a	#@17
	sta a	t0+1
	clr 	t0
	lda a	@23,x
	lda b	@22,x
	ldx 	t0
	inx 
io320044	dex 
	beq 	io320045
	asr b
	ror a
	bra 	io320044
io320045	and a	#@377
	and b	#@17
	psh a
	psh b
	jsr 	_printh
	ins 
	ins 
io329	lda a	#@17
	clr b
	ldx 	index
	and a	@23,x
	and b	@22,x
	sta a	@23,x
	sta b	@22,x
	lda a	#@12
	clr b
	cmp b	@22,x
	blt 	io330
	bgt 	io330042
	cmp a	@23,x
	bls 	io330
io330042	lda a	#@60
	bra 	io330040
io330	lda a	#@127
io330040	clr b
	add a	@23,x
	adc b	@22,x
	psh a
	psh b
	jsr 	_putchar
	ins 
	ins 
	jmp 	cret
