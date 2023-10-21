/	rp -- rp11 disk driver for bootstraps, etc.
/
/	last edit: 15-Mar-1981	D A Gwyn
/
/	prefix program.s containing
/	dska = disk starting block, ba = bus address, wc = -(word count)
/
/	prefix rrp.s or wrp.s for read or write resp.
/	(This defines entry point rblk/wblk and control word iocom)
/
/	To use: set up dska, ba, & wc; then jsr pc,[rw]blk
/	Hardware requires that wc be a multiple of 2.

RPDA = 176724			/ disk address register
SURF = 20.			/ surfaces per cylinder
SECT = 10.			/ sectors per track

	mov	dska,r1		/ lsw of disk address
	clr	r0		/ msw
	div	$SECT,r0	/ get track in r0, sector in r1
	mov	r1,-(sp)	/ save sector
	mov	r0,r1		/ lsw of track
	clr	r0		/ msw
	div	$SURF,r0	/ get cylinder in r0, surface in r1
	bisb	r1,1(sp)	/ combine track with sector
	mov	$RPDA,r1
	mov	(sp)+,(r1)	/ set up track & sector address
	mov	r0,-(r1)	/ set up cylinder address
	mov	ba,-(r1)	/ set up bus address
	mov	wc,-(r1)	/ set up -(# words to transfer)
	mov	$iocom,-(r1)	/ initiate the operation
1:
	tstb	(r1)		/ wait for "controller ready"
	bpl	1b

	rts	pc		/ transfer complete
