.text
	.globl	_set,_get,_inum,_state,_brk,_sbrk,_nd,_end
_set:
	mov	_inum,r0
	mov	r0,r1
	ash	$-2,r1
	add	_state,r1
	bic	$!3,r0
	bicb	m(r0),(r1)
	mov	r1,-(sp)
	mov	4(sp),r1
	ash	r0,r1
	bisb	r1,*(sp)+
	rts	pc
m:
	.byte	0021,0042,0104,0210
_get:
	mov	_inum,r0
	mov	r0,r1
	ash	$-2,r0
	add	_state,r0
	movb	(r0),r0
	bic	$!3,r1
	neg	r1
	ash	r1,r0
	bic	$!021,r0
	rts	pc
_brk:
	clr	_nd
_sbrk:
	mov	_nd,r0
	add	2(sp),_nd
	bit	$1,_nd
	beq	1f
	inc	_nd
1:
	sys	0;0f
	rts	pc
.data
0:	sys	break;_nd:_end
.text
