/ rx01 floppy disk driver
/ rdsect must be in 1st sector for boot, so it's not in here
rblk:
	asl     r0              
	asl     r0              / logical sector
	mov     $buf,r2
1:
	mov     r0,-(sp)
	mov     r0,r3
	clr     r1              / DEC algorithm: track= 1+ls/26
	br      3f              / skew sector= ls-23*(track-1)
2:                              / sector= 1 + 2*ss + (ls%26 >= 13)
	sub     $23.,r0
3:
	inc     r1
	sub     $26.,r3
	bpl     2b
	cmp     $-14.,r3
	rol     r0
4:
	sub     $26.,r0
	bpl     4b
	add     $27.,r0
	swab    r1
	bis     r1,r0
	jsr     pc,rdsect
	mov     (sp)+,r0
	inc     r0
	cmp     r2,$buf+512.
	blo     1b
	rts     pc
