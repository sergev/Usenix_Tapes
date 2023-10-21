**
~fildes	equ 	@4
~i	equ 	@177770
~buf	equ 	@6
~num	equ 	@10
_write	ldx 	#@7
	jsr 	csv
	clr a
	clr b
	ldx 	index
wri30001	sta a	@7,x
	sta b	@6,x
	lda a	@31,x
	lda b	@30,x
	cmp b	@6,x
	blt 	wri20001
	bgt 	wri20000
	cmp a	@7,x
	bls 	wri20001
wri20000	lda a	#@377
	tab
	bra 	wri20002
wri20001	clr a
	clr b
wri20002	tst a
	beq 	wri1
	lda a	@27,x
	lda b	@26,x
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
	add a	@27,x
	adc b	@26,x
	sta a	@27,x
	sta b	@26,x
	lda a	#@1
	clr b
	sta a	c0+1
	sta b	c0
	add a	@7,x
	adc b	@6,x
	bra 	wri30001
wri1	jmp 	cret
