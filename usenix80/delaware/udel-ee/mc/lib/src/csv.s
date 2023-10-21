* reentrant context save for m6800 c programs
* expects a,b to be useless, x to hold number
* of bytes to take off the stack.
* sets up sp to bottom of context block,
* sets index to bottom of space saved.
*
* save c0 on stack
csv	ldaa	c0+1
	ldab	c0
	psha
	pshb
* save index on stack
	ldaa	index+1
	ldab	index
	psha
	pshb
* save sp on stack
	ldaa	sp+1
	ldab	sp
	psha
	pshb
* save x0 on stack
	ldaa	x0+1
	ldab	x0
	psha
	pshb
* temp put #bytes to save in c0
* which has already been saved above
	stx	c0
* get return address and put in index (temp)
	tsx
	ldx	8,x
	stx	index
* get address of bottom context block
* and put in sp
	tsx
	dex
	stx	sp
* save t0 in place formerly occupied by
* the return address
	ldaa	t0+1
	ldab	t0
	staa	10,x
	stab	9,x
* get return address in index reg
	ldx	index
* do subtraction to set up index
	ldaa	sp+1
	ldab	sp
	suba	c0+1
	sbcb	c0
	staa	index+1
	stab	index
* set up stack pointer
	lds	index
	des
* return by jumping to return address
	jmp	0,x
