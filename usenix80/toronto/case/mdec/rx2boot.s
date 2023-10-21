/
/	RX02 Bootstrap
/
/	The RX02 bootstrap is quite complex.  First, the hardware
/	only reads in sector 1 of track 1.  In single density,
/	this means only 128 bytes of bootstrap.  Therefore, this
/	program sides in Unix block 0, which starts in physical
/	sector 1 of physical track 1 of the floppy.  It relocates
/	itself to high core, then reads in Unix blocks 498 and 499.
/	Note that because the RX211 interface is not all that easy
/	to talk to, the RX02 bootstrap device driver is rather larger
/	than the bootstrap device driver for, say, an RK05.  This
/	makes fsboot larger than one block, so it sits in blocks
/	498 and 499.  Neither this program nor fsboot may have a Unix
/	execute header on it.  The program wrboot can be used to write
/	this bootstrap onto block 0 of the floppy:
/		wrboot <rx2boot >/dev/rx0
/	The program wrx2boot will write fsboot to blocks 498 and 499
/	of the floppy:
/		wrx2boot <fsboot >/dev/rx0
/	Note that all these programs assume a single density floppy.
/	The mods for double density are fairly straight forward, but
/	many programs will have to be modified.  Fsboot should sit in
/	blocks 999 and 1000 of a double density floppy.
/
/	The block to sector mapping is intimately tied to the rx2.c driver.
/	Block 498-499 corresponds to track 0, sectors 22, 24, 26,
/	2, 4, 6, and 8.  (The last sector is unused.)
/
/	This program must be less than 128 bytes (256 for double density).
/
/	Bill Shannon - CWRU   06/05/79
/
core = 24.
.. = [core*2048.]-512.

start:
	mov	$..,sp
	mov	sp,r1
	clr	r0
	cmp	pc,r1
	bhis	2f
1:
	mov	(r0)+,(r1)+
	cmp	r1,$core*2048.
	blo	1b
	jmp	(sp)

2:
	clr	r0
	clr	r4
	mov	$22.,r1
	jsr	pc,rx2in
	jsr	pc,rx2in
	jsr	pc,rx2in
	mov	$2.,r1
	jsr	pc,rx2in
	jsr	pc,rx2in
	jsr	pc,rx2in
	jsr	pc,rx2in
	clr	pc


/
/	rx2in - read an RX02 sector.  sector in r1, track in r0,
/	address in r4.
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
	mov	$64,(r3)		/ specify word count
1:
	tstb	(r2)			/ wait for TR
	bge	1b
	mov	r4,(r3)		/ specify address
1:
	bit	$DONE,(r2)		/ wait for DONE
	beq	1b
	add	$128,r4
	add	$2,r1
	rts	pc
