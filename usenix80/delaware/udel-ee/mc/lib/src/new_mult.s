* two's complement 16 x 16  multiply
* returns least significant half of result
* in a and b regs, discards MS half.
_mult	clra
	psha
	psha
* cleared result area and a reg
	ldab	#15
	pshb
	clrb
* have initialized count and b reg cleared
	tsx
* index register points to bottom of stack
* stack looks like
* +0	COUNT
* +1	RESULT MSB
* +2	RESULT LSB
* +3	Return Address
* +4	Return Address
* +5	MULTIPLIER MSB
* +6	MULTIPLIER LSB
* +7	MULTIPLICAND MSB
* +8	MULTIPLICAND LSB
*
m100	asr	5,x
	ror	6,x
* get least sig. bit of multiplier
	bcc	m105
* not zero so add in multiplicand
	adda	8,x
	adcb	7,x
* shift result right one
m105	asrb
	rora
	ror	1,x
	ror	2,x
* done this loop yet?
	dec	0,x
	bne	m100
* msbit in 2's comp. has negative weight
* so subtract, not add
	ror	6,x
* only need to test lsbit
	bcc	m200
	suba	8,x
	sbcb	7,x
*rotate result again
m200	asrb
	ror	1,x
	ror	2,x
*discard count
	ins
* get return value
	pulb
	pula
* restore index
	ldx	index
	rts
