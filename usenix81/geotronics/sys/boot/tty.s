/	tty -- kl11/dl11 terminal driver for bootstraps, etc.
/
/	last edit: 15-Mar-1981	D A Gwyn
/
/ 1) removed UC->lc mapping;
/ 2) added test for input error (causes retry).

RCSR = 177560			/ receiver status register
RBUF = 177562			/ receiver data buffer register
XCSR = 177564			/ transmitter status register
XBUF = 177566			/ transmitter data buffer register


/	getc -- read and echo character from terminal
/		(perform normal cr/lf uc/lc mapping)
/
/	To use: jsr pc,getc	/ leaves character in R0

getc:
	tstb	*$RCSR		/ wait for receiver done
	bge	getc

	mov	*$RBUF,r0	/ get character
	bmi	getc		/ try again if error

	bic	$!177,r0	/ strip parity

	cmpb	r0,$'\r		/ map CR to NEWLINE
	bne	putc		/ echo and return
	movb	$'\n,r0
	/ fall through to "putc"


/	putc -- put a character on the terminal
/
/	To use: load R0 with character; then jsr pc,putc
/		(doesn't change R0)

putc:
	cmpb	r0,$'\n
	bne	1f		/ if regular character, just write it

	movb	$'\r,r0		/ NEWLINE echoes as CR,LF
	jsr	pc,putc		/ put the CR first
	movb	$'\n,r0
1:
	tstb	*$XCSR		/ wait for transmitter ready
	bpl	1b

	movb	r0,*$XBUF	/ write the character
	rts	pc


/	mesg -- write a string to the terminal
/
/	To use:	jsr pc,mesg; <string\0>; .even

mesg:
	movb	*(sp),r0	/ get next character
	beq	1f		/ if null, done

	jsr	pc,putc		/ write the character
	inc	(sp)		/ advance string pointer
	br	mesg
1:
	add	$2,(sp)		/ adjust return pc to skip string
	bic	$1,(sp)
	rts	pc		/ return to instruction after string


/	callout -- remove exe header if necessary, then jump to 0
/
/	To use: read code into memory starting at loc. 0; then jmp callout

callout:
	clr	r0		/ -> loc. 0
	cmp	(r0),$407	/ a.out header?
	bne	2f
1:
	mov	20(r0),(r0)+	/ move everything down 16. bytes
	cmp	r0,sp		/ stop before bootstrap is moved
	blo	1b
2:
	mov	$..,-(sp)	/ so that rts pc will restart bootstrap
	clr	pc		/ jump to 0
