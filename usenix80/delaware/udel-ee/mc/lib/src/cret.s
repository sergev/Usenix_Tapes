* context restore for m6800 c programs
*
* This maintains return value in a,b registers
* and restores index register
*
* get pointer to context block
cret	lds	sp
* restore old sp
	tsx
	ldx	2,x
	stx	sp
* restore old c0
	tsx
	ldx	6,x
	stx	c0
* restore old t0
	tsx
	ldx	8,x
	stx	t0
* restore old x0
	tsx
	ldx	0,x
	stx	x0
* restore old index
* this is done last so that
* the return will be made with
* index register valid
	tsx
	ldx	4,x
	stx	index
* pop all the stuff off the stack
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
* and return
	rts
