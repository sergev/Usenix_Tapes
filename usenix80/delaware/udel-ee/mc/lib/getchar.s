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
get2	ldx 	index
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
	bne 	get20001
	cmp a	1,x
	bne 	get20001
	lda a	#@377
	bra 	get20002
get20001	clr a
get20002	ins 
	ins 
	tst a
	bne 	get2
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
	bne 	get4
	lda a	#@12
	sta a	@6,x
get4	lda a	@6,x
	clr b
	psh a
	psh b
	jsr 	_putchar
	ins 
	ins 
	lda a	@6,x
	clr b
	jmp 	cret
