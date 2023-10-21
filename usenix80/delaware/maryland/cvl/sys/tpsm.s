.globl _smdate, cerror

_smdate:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),0f	/name
	mov	6(r5),r1	/pointer to time
	mov	(r1)+,r0	/time0
	mov	(r1),r1		/time1
	sys	0; 9f
	bec 1f
	jmp	cerror
1:
	clr r0
	mov	(sp)+,r5
	rts	pc
.data
9:
	sys	30.; 0:..
