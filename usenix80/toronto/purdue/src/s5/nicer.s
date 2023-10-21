/ C library -- nicer
/ error = nicer(pid, hownice)

.globl	_nicer, cerror
nicer = 51.
indir = 0

_nicer:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(sp),r0
	mov	6(sp),8f
	sys	indir; 9f
	bec	1f
	jmp	cerror
1:
	clr	r0
	mov	(sp)+,r5
	rts	pc

.data
9:
	sys	nicer; 8:..
