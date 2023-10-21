276a
4:
.
269a
	movb	(r2)+,r0
	bic	$177400,r0	/ remove high byte
	jsr	r5,code
		<.byte %o\n\0>; .even
		r0
	dec	(sp)
	bne	3b
	tst	(sp)+
	br	4f
6:
	asr	r0
	mov	r0,-(sp)
	mov	6(r3),r2
3:
.
266c
	bit	$1,r0		/ odd?
	beq	6f		/ no, word data
.
181a
6:
.
180a
	sub	$1,-4(r4)		/ reduce const length
	br	3f
4:
	cmp	holquo,$int1con+10	/ simple int*1?
	bne	4f
	sub	$3,-4(r4)		/ reduce const length
	br	6f
4:
	cmp	holquo,$int2con+10	/ simple int*2?
	bne	4f
.
179c
	cmp	holquo,$log1con+10	/ simple log*1?
.
123,124c
/	inc	r0
/	bic	$1,r0
.
87,88c
/	incb	holquo+1		/ round size
/	bicb	$1,holquo+1
.
4a
/	modified 05-May-1980 by D A Gwyn:
/	1) allow data'ing 1-byte quantities.
.
w
q
