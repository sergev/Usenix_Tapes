/ PO library -- xrel

/ error =  xrel(file);

.globl	_xrel, cerror

_xrel:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),r0
	sys	xrel
	bec	1f
	jmp	cerror
1:
	clr	r0
	mov	(sp)+,r5
	rts	pc
xrel = 63.
