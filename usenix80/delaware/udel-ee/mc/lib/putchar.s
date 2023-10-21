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
put2	ldx 	index
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
	bne 	put20001
	cmp a	1,x
	bne 	put20001
	lda a	#@377
	bra 	put20002
put20001	clr a
put20002	ins 
	ins 
	tst a
	bne 	put2
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
	bne 	put20005
	lda a	#@377
	tab
	bra 	put20006
put20005	clr a
	clr b
put20006	tst a
	beq 	put1
	lda a	#@15
	clr b
	psh a
	psh b
	jsr 	_putchar
	ins 
	ins 
put1	jmp 	cret
