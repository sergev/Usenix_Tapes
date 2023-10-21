/
/	rrx2 - read RX02 block for fsboot
/

/ paramters for single density
NSPB = 4
NWPS = 64.
NBPS = 128.

rblk:
	mov	r4,-(sp)		/ save r4
	mov	r3,-(sp)		/ and r3
	mov	r2,-(sp)		/ and r2
	mov	ba,adr			/ get us a copy of ba
	mov	dska,r0			/ convert disk block number
	ash	$2,r0			/   to logical sector number
	mov	r0,sector		/ save it
	mov	$NSPB,r4		/ loop count, number sectors per block
9:
	jsr	pc,factor		/ convert logical sector to physical
	jsr	pc,rx2in		/ read the physical sector
	inc	sector			/ bump to next logical sector
	add	$NBPS,adr		/ bump to next address
	sob	r4,9b
	mov	r2,(sp)+		/ restore regs
	mov	r3,(sp)+
	mov	r4,(sp)+
	rts	pc


/
/	factor - convert logical sector to physical track, sector.
/	hand optimized from rx2factor in rx2.c driver.
/	RT-11 compatible mapping.
/

factor:
	mov	sector,r1
	clr	r0
	div	$26.,r0
	mov	r0,r3
	asl	r1
	cmp	$26.,r1
	jgt	1f
	inc	r1
1:
	clr	r0
	div	$26.,r0
	mov	r1,r2
	mov	r3,r1
	mul	$6,r1
	add	r2,r1
	clr	r0
	div	$26.,r0
	inc	r1			/ physical sector is here
	inc	r3
	cmp	$77.,r3
	jgt	1f
	clr	r3
1:
	mov	r3,r0			/ physical track is here
	rts	pc


/
/	rx2in - read an RX02 sector.  sector in r1, track in r0,
/	address in adr.
/

RXCS = 177170
RXDB = 177172
READ = 7
EMPTY = 3
DONE = 40

rx2in:
	mov	$RXCS,r2
	mov	$RXDB,r3
	mov	$READ,(r2)		/ do read function
1:
	tstb	(r2)			/ wait for TR
	bge	1b
	mov	r1,(r3)			/ specify sector
1:
	tstb	(r2)			/ wait for TR
	bge	1b
	mov	r0,(r3)			/ specify track
1:
	bit	$DONE,(r2)		/ wait for DONE
	beq	1b
	mov	$EMPTY,(r2)		/ empty sector buffer
1:
	tstb	(r2)			/ wait for TR
	bge	1b
	mov	$NWPS,(r3)		/ specify word count
1:
	tstb	(r2)			/ wait for TR
	bge	1b
	mov	adr,(r3)		/ specify address
1:
	bit	$DONE,(r2)		/ wait for DONE
	beq	1b
	rts	pc


sector: 0
adr: 0
