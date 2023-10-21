// C library -- halt

// error = halt()

.globl	_errno
.globl	_halt
.text

_halt:	sys	59.
	mov	r0,_errno
	mov	$-1,r0
	bes	err
	inc	r0
err:	rts	pc
