/	showps - show a string for 'ps'
/
/	call:
/	showps("string1","string2",...,"stringn",0);
/
/	Showps moves the string arguments given to the high end of
/	the users stack area, where they are visable to the world
/	via 'ps'.  Only the first 62 bytes are moved, any remaining
/	bytes are discarded.  Showps MUST be called first from main
/	since this first call shifts the user's stack down and patches
/	argv and argv[] to point to their shifted positions.  Its
/	patching technique is merely to subtract the stack shift value
/       from each pointer, so if you are altering these yourself it's wise
/       to call showps first.  After the first call showps may be called
/       from anywhere.

.globl	_showps
shift = 64.

_showps:
	tstb    9f
	bne	7f		/if stack initialized
	incb    9f
	mov	sp,r0
	sub	$shift,sp	/shift stack down
	sub	$shift,r5
	mov	sp,r1
5:
	mov	(r0)+,(r1)+
	tst	r0
	bne	5b
	mov	r5,r0		/add shift value to argv etc.
	add	$6,r0		/skip: r5save returnaddr argc
6:
	cmp	$-1,(r0)
	beq	7f
	sub	$shift,(r0)+
	br	6b
7:
	mov	$-shift,r1	/r1='ps' area at top of memory
	mov	$-1,(r1)+	/set start flag for ps
	mov	r5,-(sp)
	mov	sp,r5
	add	$4,r5
1:
	mov	(r5)+,r0	/get next user string pointer
	beq	3f
2:
	tst	r1
	beq	4f		/if no room left
	movb	(r0)+,(r1)+
	bne	2b
	br	1b		/if end of string
3:
	tst	r1		/zero fill
	beq	4f
	clrb	(r1)+
	br	3b
4:
	mov	(sp)+,r5
	rts	pc

.bss
9:      .=.+1
