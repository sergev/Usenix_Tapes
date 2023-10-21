**
~ap	equ 	@4
~c	equ 	@177766
~f	equ 	@177762
~n	equ 	@177770
~p	equ 	@177764
_atoi	ldx 	#@15
	jsr 	csv
	ldx 	index
	lda a	@33,x
	lda b	@32,x
	sta a	@11,x
	sta b	@10,x
	clr a
	clr b
	sta a	@15,x
	sta b	@14,x
	sta a	@7,x
	sta b	@6,x
atoi3	lda a	@11,x
	lda b	@10,x
	sta a	c0+1
	sta b	c0
	ldx 	c0
	lda a	x
	clr b
	psh a
	psh b
	lda a	#@40
	tsx 
	cmp b	@0,x
	bne 	atoi20001
	cmp a	1,x
	bne 	atoi20001
	lda a	#@377
	tab
	bra 	atoi20002
atoi20001	clr a
	clr b
atoi20002	ins 
	ins 
	psh a
	psh b
	ldx 	index
	lda a	@11,x
	lda b	@10,x
	sta a	c0+1
	sta b	c0
	ldx 	c0
	lda a	x
	clr b
	psh a
	psh b
	lda a	#@11
	tsx 
	cmp b	@0,x
	bne 	atoi20004
	cmp a	1,x
	bne 	atoi20004
	lda a	#@377
	tab
	bra 	atoi20005
atoi20004	clr a
	clr b
atoi20005	ins 
	ins 
	tst a
	bne 	atoi20007
	tst b
	bne 	atoi20007
	pul b
	pul a
	des 
	des 
	tst a
	bne 	atoi20007
	tst b
	bne 	atoi20007
	clr a
atoi20007	ins 
	ins 
	tst a
	beq 	atoi4
	lda a	#@1
	clr b
	sta a	c0+1
	sta b	c0
	ldx 	index
atoi30003	add a	@11,x
	adc b	@10,x
	sta a	@11,x
	sta b	@10,x
	jmp 	atoi3
atoi4	ldx 	index
	lda a	@11,x
	lda b	@10,x
	sta a	c0+1
	sta b	c0
	ldx 	c0
	lda a	x
	clr b
	psh a
	psh b
	lda a	#@55
	tsx 
	cmp b	@0,x
	bne 	atoi20010
	cmp a	1,x
	bne 	atoi20010
	lda a	#@377
	bra 	atoi20011
atoi20010	clr a
atoi20011	ins 
	ins 
	tst a
	beq 	atoi6
	lda a	#@1
	clr b
	sta a	c0+1
	sta b	c0
	ldx 	index
	add a	@7,x
	adc b	@6,x
	sta a	@7,x
	sta b	@6,x
	lda a	#@1
	clr b
	sta a	c0+1
	sta b	c0
	bra 	atoi30003
atoi6	ldx 	index
	lda a	@11,x
	lda b	@10,x
	sta a	c0+1
	sta b	c0
	ldx 	c0
	lda a	x
	clr b
	psh a
	psh b
	lda a	#@60
	tsx 
	cmp b	@0,x
	bgt 	atoi20014
	blt 	atoi20013
	cmp a	1,x
	bhi 	atoi20014
atoi20013	lda a	#@377
	tab
	bra 	atoi20015
atoi20014	clr a
	clr b
atoi20015	ins 
	ins 
	psh a
	psh b
	ldx 	index
	lda a	@11,x
	lda b	@10,x
	sta a	c0+1
	sta b	c0
	ldx 	c0
	lda a	x
	clr b
	psh a
	psh b
	lda a	#@71
	tsx 
	cmp b	@0,x
	blt 	atoi20017
	bgt 	atoi20016
	cmp a	1,x
	bcs 	atoi20017
atoi20016	lda a	#@377
	tab
	bra 	atoi20018
atoi20017	clr a
	clr b
atoi20018	ins 
	ins 
	tst a
	bne 	atoi20019
	tst b
	beq 	atoi20020
atoi20019	pul b
	pul a
	des 
	des 
	tst a
	bne 	atoi20021
	tst b
	bne 	atoi20021
atoi20020	clr a
	bra 	atoi20022
atoi20021	lda a	#@377
atoi20022	ins 
	ins 
	tst a
	beq 	atoi7
	lda a	#@12
	clr b
	psh a
	psh b
	ldx 	index
	lda a	@15,x
	lda b	@14,x
	psh a
	psh b
	jsr 	mult
	ins 
	ins 
	ins 
	ins 
	psh a
	psh b
	lda a	#@1
	clr b
	sta a	c0+1
	sta b	c0
	add a	@11,x
	adc b	@10,x
	sta a	@11,x
	sta b	@10,x
	sub a	c0+1
	sbc b	c0
	sta a	c0+1
	sta b	c0
	ldx 	c0
	lda a	x
	clr b
	tsx 
	add a	1,x
	adc b	x
	ins 
	ins 
	add a	#@320
	adc b	#@377
	ldx 	index
	sta a	@15,x
	sta b	@14,x
	jmp 	atoi6
atoi7	ldx 	index
	lda a	@7,x
	tst a
	beq 	atoi8
	lda a	@15,x
	lda b	@14,x
	com b
	com a
	add a	#@1
	adc b	#@0
	sta a	@15,x
	sta b	@14,x
atoi8	lda a	@15,x
	lda b	@14,x
	jmp 	cret
