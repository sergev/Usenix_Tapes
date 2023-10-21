/ rk06 or rk07 disk driver

hktype = 0	/ 2000 for rk07 vs 0 for rk06
hkcs1 = 177440
hkda  = 177446
hkcs2 = 177450
hkca  = 177460

packn = 021
clear = 040

first = .+2	/ dirty, but i need the space
	tst	$0
	bne	1f
	mov	$clear,*$hkcs2
	mov	$packn+hktype,*$hkcs1
	inc	first
1:
	mov	dska,r1
	clr	r0
	div	$22.,r0
	mov	r1,-(sp)
	mov	r0,r1
	clr	r0
	div	$3.,r0
	bisb	r1,1(sp)
	mov	r0,*$hkca
	mov	$hkda,r1
	mov	(sp)+,(r1)
	mov	ba,-(r1)
	mov	wc,-(r1)
	mov	$iocom+hktype,-(r1)
1:
	bit	$200,(r1)
	beq	1b
	rts	pc
