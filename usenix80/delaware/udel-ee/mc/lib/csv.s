csv	ldaa	c0+1
	ldab	c0
	psha
	pshb
	ldaa	index+1
	ldab	index
	psha
	pshb
	ldaa	sp+1
	ldab	sp
	psha
	pshb
	ldaa	x0+1
	ldab	x0
	psha
	pshb
	stx	c0
	tsx
	ldx	8,x
	stx	index
	tsx
	dex
	stx	sp
	ldaa	t0+1
	ldab	t0
	staa	10,x
	stab	9,x
	ldx	index
	ldaa	sp+1
	ldab	sp
	suba	c0+1
	sbcb	c0
	staa	index+1
	stab	index
	lds	index
	des
	jmp	0,x
cret	lds	sp
	tsx
	ldx	2,x
	stx	sp
	tsx
	ldx	6,x
	stx	c0
	tsx
	ldx	8,x
	stx	t0
	tsx
	ldx	0,x
	stx	x0
	tsx
	ldx	4,x
	stx	index
	ins
	ins
	ins
	ins
	ins
	ins
	ins
	ins
	ins
	ins
	rts
