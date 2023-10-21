/	rk -- rk11 disk driver for bootstraps, etc.
/
/	last edit: 15-Mar-1981	D A Gwyn
/
/	prefix program.s containing
/	dska = disk starting block, ba = bus address, wc = -(word count)
/
/	prefix rrk.s or wrk.s for read or write resp.
/	(This defines entry point rblk/wblk and control word iocom)
/
/	To use: set up dska, ba, & wc; then jsr pc,[rw]blk

RKDA = 177412			/ disk address register
SECT = 12.			/ sectors per track

	mov	dska,r1		/ lsw of disk address
	clr	r0		/ msw
	div	$SECT,r0	/ get track in r0, sector in r1
	ash	$4.,r0		/ bits 15..4 are drive, cylinder, & surface
	bis	r1,r0		/ bits 3..0 are sector
	mov	$RKDA,r1
	mov	r0,(r1)		/ set up address
	mov	ba,-(r1)	/ set up bus address
	mov	wc,-(r1)	/ set up - (# words to transfer)
	mov	$iocom,-(r1)	/ initiate the operation
1:
	tstb	(r1)		/ wait for "controller ready"
	bpl	1b

	rts	pc		/ transfer complete
