pri15	fcb 	@55,@63,@62,@67,@66,@70
	fcb 	@0
pri20	fcb 	@156,@165,@154,@154,@0
**
~argp	equ 	@177764
~argv	equ 	@6
~dnum	equ 	@177766
~format	equ 	@4
~strp	equ 	@177770
~delay	equ 	@177762
pri20020	fdb	pri15
pri20026	fdb	pri20
pri20029	fdb 	pri10002-pri10001
pri10001	fdb 	@143
	fdb 	@144
	fdb 	@150
	fdb 	@157
	fdb 	@163
pri10002	rmb 	2
	fdb	pri11
	fdb	pri12
	fdb	pri17
	fdb	pri16
	fdb	pri18
	fdb	pri30022
_printf	ldx 	#@15
	jsr 	csv
	clr a
	clr b
	ldx 	index
pri30024	sta a	@7,x
	sta b	@6,x
	lda a	#@160
	lda b	#@27
	cmp b	@6,x
	blt 	pri3
	bgt 	pri30025
	cmp a	@7,x
	bls 	pri3
pri30025	lda a	#@1
	clr b
	sta a	c0+1
	sta b	c0
	add a	@7,x
	adc b	@6,x
	bra 	pri30024
pri3	lda a	#@34
	clr b
	add a	index+1
	adc b	index
	sta a	@11,x
	sta b	@10,x
pri5	lda a	@33,x
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
	bne 	pri20004
	cmp a	1,x
	beq 	pri20005
pri20004	lda a	#@377
	tab
	bra 	pri20006
pri20005	clr a
	clr b
pri20006	ins 
	ins 
	tst a
	bne 	pri30028
	jmp 	pri1
pri30028	ldx 	index
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
	bne 	pri20008
	cmp a	1,x
	beq 	pri20009
pri20008	lda a	#@377
	bra 	pri20010
pri20009	clr a
pri20010	ins 
	ins 
	tst a
	beq 	pri7
pri30022	ldx 	index
	lda a	@33,x
	lda b	@32,x
	sta a	c0+1
	sta b	c0
	ldx 	c0
	lda a	x
	clr b
pri30006	psh a
	psh b
	jsr 	_putchar
pri30008	ins 
	ins 
	jmp 	pri8
pri7	lda a	#@1
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
	jmp 	pri10
pri11	lda a	#@2
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
	bra 	pri30006
pri12	lda a	#@2
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
	blt 	pri13
	bgt 	pri30026
	cmp a	@13,x
	bls 	pri13
pri30026	lda a	@13,x
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
	blt 	pri14
	bgt 	pri30027
	cmp a	@13,x
	bls 	pri14
pri30027	lda a	pri20020+1
	lda b	pri20020
	psh a
	psh b
	jsr 	_printf
	bra 	pri30029
pri14	lda a	#@55
	clr b
	psh a
	psh b
	jsr 	_putchar
	ins 
	ins 
pri13	lda a	@13,x
	lda b	@12,x
	psh a
	psh b
	jsr 	_printd
pri30029	bra 	pri30030
pri16	lda a	#@2
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
pri30030	bra 	pri30031
pri17	lda a	#@2
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
pri30031	jmp 	pri30008
pri18	lda a	#@2
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
	bne 	pri20023
	cmp a	1,x
	bne 	pri20023
	lda a	#@377
	bra 	pri20024
pri20023	clr a
pri20024	ins 
	ins 
	tst a
	beq 	pri21
	lda a	pri20026+1
	lda b	pri20026
	psh a
	psh b
	jsr 	_printf
pri30001	ins 
	ins 
pri21	ldx 	index
	lda a	@15,x
	lda b	@14,x
	sta a	c0+1
	sta b	c0
	ldx 	c0
	lda a	x
	tst a
	beq 	pri8
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
	bra 	pri30001
pri10	sta b	pri10002
	sta a	pri10002+1
	ldx 	#pri10001-2
pri10003	inx 
	inx 
	cmp a	1,x
	bne 	pri10003
	cmp b	x
	bne 	pri10003
	inx 
	inx 
	stx 	c0
	lda a	c0+1
	lda b	c0
	add a	pri20029+1
	adc b	pri20029
	sta a	c0+1
	sta b	c0
	ldx 	c0
	ldx 	x
	jmp 	x
pri8	lda a	#@1
	clr b
	sta a	c0+1
	sta b	c0
	ldx 	index
	add a	@33,x
	adc b	@32,x
	sta a	@33,x
	sta b	@32,x
	jmp 	pri5
pri1	jmp 	cret
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
	jsr 	div
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
	bne 	pri20030
	cmp a	1,x
	beq 	pri20031
pri20030	lda a	#@377
	bra 	pri20032
pri20031	clr a
pri20032	ins 
	ins 
	tst a
	beq 	pri25
	ldx 	index
	lda a	@7,x
	lda b	@6,x
	psh a
	psh b
	jsr 	_printd
	ins 
	ins 
pri25	lda a	#@12
	clr b
	psh a
	psh b
	ldx 	index
	lda a	@7,x
	lda b	@6,x
	psh a
	psh b
	jsr 	mult
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
	bne 	pri30032
	cmp a	@23,x
	beq 	pri27
pri30032	lda a	#@3
	and a	#@17
	sta a	t0+1
	clr 	t0
	lda a	@23,x
	lda b	@22,x
	ldx 	t0
	inx 
pri20038	dex 
	beq 	pri20039
	asr b
	ror a
	bra 	pri20038
pri20039	and a	#@377
	and b	#@37
	psh a
	psh b
	jsr 	_printo
	ins 
	ins 
pri27	lda a	#@7
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
	bne 	pri30041
	cmp a	@23,x
	beq 	pri29
pri30041	lda a	#@4
	and a	#@17
	sta a	t0+1
	clr 	t0
	lda a	@23,x
	lda b	@22,x
	ldx 	t0
	inx 
pri20044	dex 
	beq 	pri20045
	asr b
	ror a
	bra 	pri20044
pri20045	and a	#@377
	and b	#@17
	psh a
	psh b
	jsr 	_printh
	ins 
	ins 
pri29	lda a	#@17
	clr b
	ldx 	index
	and a	@23,x
	and b	@22,x
	sta a	@23,x
	sta b	@22,x
	lda a	#@12
	clr b
	cmp b	@22,x
	blt 	pri30
	bgt 	pri30042
	cmp a	@23,x
	bls 	pri30
pri30042	lda a	#@60
	bra 	pri30040
pri30	lda a	#@127
pri30040	clr b
	add a	@23,x
	adc b	@22,x
	psh a
	psh b
	jsr 	_putchar
	ins 
	ins 
	jmp 	cret
