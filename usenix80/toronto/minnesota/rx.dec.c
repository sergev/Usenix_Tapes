#
/*
 * RX11 floppy disk driver
 *
 * Modified 15 DEC 77 by SSB,DWD,BEB, University of Minnesota 
 * for RX11 floppy drive.
 * Modified 2/1/78, Brad Blasing, to emulate DMA operations.
 * Modified 6/16/78, Brad Blasing, to provide better error recovery
 *  (we don't loop at spl5 waiting for INIT to finish any more).
 * Modified 5/79, SICL, to work with the slow slow slow DEC RX drive.
 *
 */

#include "../param.h"
#include "../buf.h"
#include "../conf.h"
#include "../user.h"

/*  #define DMA	1    turn on for dma driver */
#define	DEBUG	1
#define RXADDR	0177170
#define NRXTYP	3
#define MAXRETRY  5
#define TTIME	60	/* Timeout time in HZ */
#define RRATE	6	/* Recall rate for rxreset */
#define RESETMAX 100	/* Max. num. of reset recalls before timeout */
			/* RESETMAX*RRATE/60 = time in second */

/* rxcs commands and masks */
#define INIT	0040000
#define ERROR	0100000
#define IENABLE	0100
#define TR	0200
#define DONE	040
#define GO	001
#define FILL	000
#define EMPTY	002
#define WRITE	004
#define READ	006
#define RDSTAT	012
#define RDERR	016

/* status register masks */
#define DR	0200	/* Drive Ready */

#define WAITTR	waittr();	/* while((RXADDR->rxcs & TR) != 0); */
#define KISA	0172340
#define KISD	0172300

struct {
	int rxcs;
	int rxdb;
};

struct	rxtype {
	int	secsize;		/* size (bytes) one sector */
	int	secpertrk;		/* sectors/track */
	int	secperblk;		/* sectors/unix block */
	int	numsec;			/* total sectors on device */
	int	numblks;		/* number of blocks on device */
	int	secincr;		/* increment to get next sector of block */
	int	intrlv;			/* interleaving factor */
	int	skew;			/* sector skew across tracks */
	int	trkoffset;		/* offset num of 1st sec */
} rxtypes[NRXTYP] {
		128, 26, 4, 77*26, 500, 2, 13, 6, 0,	/* our "standard" format */
		128, 26, 4, 77*26, 500, 1, 26, 0, 0,	/* IBM format */
		128, 26, 4, 76*26, 494, 2, 13, 6, 1	/* Terak or RT11 format */
};

struct	rxstat {
	int	fminor;			/* present request device number */
	struct	rxtype *ftype;		/* present request floppy type */
	int	bytect;			/* remaining bytes (neg) */
	int	sector;			/* absolute sector (0..numsec-1) */
	int	toutact;		/* timeout active */
	int	reqnum;			/* floppy request number for timeout */
	char	*coreaddr;		/* current core address for transfer */
#ifdef DMA
	char	*coreblk;		/* block no. to put in seg. register */
#endif
} rxstat;

struct	devtab	rxtab;

waittr() {
int delay;

	delay = 0;
	while(!(RXADDR->rxcs & TR) && --delay);
	if(delay == 0 && SW->integ == 0177777)
		printf("TR timeout.\n");
}
rxstrategy(abp)
struct buf *abp;
{
	register struct buf *bp;
	extern int rxtimeout();

#ifdef DEBUG
	if(abp->b_dev.d_minor == 127) {
		rxdebug();
		iodone(abp);
		spl0();
		return;
	}
#endif
	bp = abp;
	if(bp->b_flags&B_PHYS)
		/*
		 * only valid for an 11/70
		 */
		mapalloc(bp);
	/*
	 * test for valid request
	 */
	if(rxok(bp) == 0) {
		bp->b_flags =| B_ERROR;
		iodone(bp);
		return;
	}
	/*
	 * link buffer into device queue
	 */
	bp->av_forw = 0;
	spl5();
	if(rxtab.d_actf == 0)
		rxtab.d_actf = bp;
	else
		rxtab.d_actl->av_forw = bp;
	rxtab.d_actl = bp;
	/*
	 * start rxtimeout if inactive
	 */
	if(rxstat.toutact == 0) {
		rxstat.toutact++;
		timeout(rxtimeout, 0, TTIME);
	}
	/*
	 * start device if there is no current request
	 */
	if(rxtab.d_active == 0)
		rxstart();
	spl0();
}

rxstart()
{
	register struct buf *bp;
	register int minor;

	/*
	 * if there is no request in queue...return
	 */
loop:	if((bp = rxtab.d_actf) == 0)
		return;
	/*
	 * check if drive ready
	 */
	minor = (bp->b_dev.d_minor & 1) << 4;
	RXADDR->rxcs = minor | RDSTAT | GO;
	while((RXADDR->rxcs & DONE) == 0) ;
	if((RXADDR->rxdb & DR) == 0) {
		prdev("Floppy not ready", bp->b_dev);
		rxabtbuf();
		goto loop;
	}
	/*
	 * set active request flag
	 */
	rxtab.d_active++;
	rxsetup(bp);
	rxregs(bp);
}

rxintr()
{
	register struct buf *bp;
	register struct rxtype *rxt;

	/*
	 * if there is no active request, false alarm.
	 */
	if(rxtab.d_active == 0) 
		return;
	rxtab.d_active = 0;
	/*
	 * pointer to the buffer
	 */
	bp = rxtab.d_actf;
	/*
	 * pointer to a data structure describing
	 *  the type of device (i.e. interleaving)
	 */
	rxt = rxstat.ftype;
	/*
	 * check error bit
	 */
	if(RXADDR->rxcs & ERROR) {
		/*
		 * send read error register command
		 */
		RXADDR->rxcs = RDERR | GO;
		deverror(bp, RXADDR->rxcs, RXADDR->rxdb);	
		/*
		 * make MAXRETRY retries on an error
		 */
		if(++rxtab.d_errcnt <= MAXRETRY) {
			rxreset(0);
			return;
		}
		/*
		 * return an i/o error
		 */
		bp->b_flags =| B_ERROR;
	} else {
		/*
		 * if we just read a sector, we need to
		 *  empty the device buffer
		 */
		if(bp->b_flags & B_READ) 
			rxempty(bp);
		/*
		 * see if there is more data to read for
		 * this request.
		 */
		rxstat.bytect =+ rxt->secsize;
		rxstat.sector++;
		if(rxstat.bytect < 0 && rxstat.sector < rxt->numsec) {
			rxtab.d_active++;
			rxregs(bp);
			return;
		}
	}
	rxtab.d_errcnt = 0;
	/*
	 * unlink block from queue
	 */
	rxtab.d_actf = bp->av_forw;
	iodone(bp);
	/*
	 * start i/o on next buffer in queue
	 */
	rxstart();
}

rxreset(flag)
{
	/*
	 * Check to see if this is a call from rxintr or
	 * a recall from timeout.
	 */
	if(flag) {
		if(RXADDR->rxcs & DONE) {
			rxtab.d_active = 0;
			rxstart();
		} else
			if(flag > RESETMAX) {
				prdev("Reset timeout", rxtab.d_actf->b_dev);
				rxabtbuf();
				rxstart();
			} else {
				timeout(rxreset, flag+1, RRATE);
				/*
				 * Keep rxtimeout from timing out.
				 */
				rxstat.reqnum++;
			}
	} else {
		RXADDR->rxcs = INIT;
		rxtab.d_active++;
		rxstat.reqnum++;
		timeout(rxreset, 1, 1);
	}
}

rxregs(abp)
struct buf *abp;
{
	register struct buf *bp;
	register int minor;
	register struct rxtype *rxt;
	int	cursec, curtrk;

	/*
	 * set device bit into proper position for command
	 */
	minor = rxstat.fminor << 4;
	bp = abp;
	rxt = rxstat.ftype;
	/*
	 * increment interrupt request number
	 */
	rxstat.reqnum++;
	/*
	 * if command is read, initiate the command
	 */
	if(bp->b_flags & B_READ){
		RXADDR->rxcs = minor | IENABLE | GO | READ;
	} else {
		/*
		 * if command is write, fill the device buffer,
		 *   then initiate the write
		 */
		rxfill(bp);
		RXADDR->rxcs = minor | IENABLE | GO | WRITE;
	}
	/*
	 * set track number
	 */
	curtrk = rxstat.sector / rxt->secpertrk;
	/*
	 * set sector number
	 */
	minor = rxstat.sector % rxt->secpertrk;
	cursec = (minor % rxt->intrlv) * rxt->secincr +
		(minor / rxt->intrlv);
	/*
	 * add skew to sector
	 */
	if(rxt->skew != 0)
		cursec = (cursec + curtrk * rxt->skew) %
			rxt->secpertrk;
	/*
	 * massage registers
	 */
	WAITTR
	RXADDR->rxdb = cursec + 1;
	WAITTR
	RXADDR->rxdb = curtrk + rxt->trkoffset;
}

rxok(abp)
struct buf *abp;
{
	register struct buf *bp;
	register int type;
	register int minor;

	/*
	 * get sub-device number and type from minor device number
	 */
	minor = (bp = abp)->b_dev.d_minor;
	type = minor >> 3;
	/*
	 * check for valid type
	 *
	 * check for block number within range of device
	 */
	if(type >= NRXTYP ||
		bp->b_blkno >= rxtypes[type].numblks)
		return(0);
	return(1);
}

rxsetup(abp)
struct buf *abp;
{
	register struct buf *bp;
	register int minor;
	register struct rxtype *rxt;

	/*
	 * get minor device number from buffer
	 */
	minor = (bp = abp)->b_dev.d_minor;
	/*
	 * get sub-device number from minor device number
	 */
	rxt = rxstat.ftype = &rxtypes[minor >> 3];
	/*
	 * make sure device number is only 0 or 1
	 */
	rxstat.fminor = minor & 1;
	/*
	 * get byte count to read from buffer (negative number)
	 */
	rxstat.bytect = (bp->b_wcount << 1);
	/*
	 * transform block number into the first
	 * sector to read on the floppy
	 */
	rxstat.sector = bp->b_blkno * rxt->secperblk;
	/*
	 * set the core address to get or put bytes.
	 */
#ifndef DMA
	rxstat.coreaddr = bp->b_addr;
#endif
#ifdef DMA
	rxstat.coreaddr = (bp->b_addr & 077) + 0120000;
	rxstat.coreblk = ((bp->b_addr >> 6) & 01777) |
		((bp->b_xmem & 03) << 10);
#endif
}

#ifndef DMA
rxempty(abp)
struct buf *abp;
{
	register int i;
	register char *cp;
	register int wc;

	/*
	 * start empty buffer command
	 */
	RXADDR->rxcs = EMPTY | GO;
	/*
	 * get core address and byte count
	 */
	cp = rxstat.coreaddr;
	rxstat.coreaddr =+ 128;
	wc = ((rxstat.bytect <= -128)? 128 : -rxstat.bytect);
	/*
	 * move wc bytes from the device buffer
	 *   into the in core buffer
	 */
	for(i=wc; i>0; --i) {
		WAITTR
		*cp++ = RXADDR->rxdb;
	}
	/*
	 * sluff excess bytes
	 */
	for(i=128-wc; i>0; --i) {
		WAITTR
		cp = RXADDR->rxdb;
	}
}

rxfill(abp)
struct buf *abp;
{
	register int i;
	register char *cp;
	register int wc;

	/*
	 * initiate the fill buffer command
	 */
	RXADDR->rxcs = FILL | GO;
	/*
	 * get core address and byte count
	 */
	cp = rxstat.coreaddr;
	rxstat.coreaddr =+ 128;
	wc = ((rxstat.bytect <= -128)? 128 : -rxstat.bytect);
	/*
	 * move wc bytes from the in-core buffer to
	 *   the device buffer
	 */
	for(i=wc;  i>0; --i) {
		WAITTR
		RXADDR->rxdb = *cp++;
	}
	/*
	 * sluff excess bytes
	 */
	for(i=128-wc; i>0; --i) {
		WAITTR
		RXADDR->rxdb = 0;
	}
}
#endif

#ifdef DMA
	/*
	 * This copy of the fill and empty routines emulate a dma
	 * floppy controller.  It adds the feature of being able
	 * to write anywhere in physical memory, just like an rk
	 * disk.  To do this, we borrow a segmentation register
	 * to do the transfer.  While the segmentation register
	 * is pointing to the proper place, we need to run at spl7.
	 * This is harder on the system, so the non-dma driver should
	 * be used if you only intend to do buffer requests (i.e.
	 * no swapping or raw i/o).
	 */

struct { int r[]; };

rxempty(abp)
struct buf *abp;
{
	register int i;
	register char *cp;
	register int wc;
	int a,d;

	/*
	 * start empty buffer command
	 */
	RXADDR->rxcs = EMPTY | GO;
	/*
	 * get core address and byte count
	 */
	cp = rxstat.coreaddr;
	wc = ((rxstat.bytect <= -128)? 128 : -rxstat.bytect);
	/*
	 * save and set segmentation register.
	 */
	a = KISA->r[5];
	d = KISD->r[5];
	spl7();
	KISA->r[5] = rxstat.coreblk;
	KISD->r[5] = 01006;
	/*
	 * move wc bytes from the device buffer
	 *   into the in core buffer
	 */
	for(i=wc; i>0; --i) {
		WAITTR
		*cp++ = RXADDR->rxdb;
	}
	/*
	 * sluff excess bytes
	 */
	for(i=128-wc; i>0; --i) {
		WAITTR
		cp = RXADDR->rxdb;
	}
	KISA->r[5] = a;
	KISD->r[5] = d;
	spl5();
	rxstat.coreblk =+ 2;
}

rxfill(abp)
struct buf *abp;
{
	register int i;
	register char *cp;
	register int wc;
	int a,d;

	/*
	 * initiate the fill buffer command
	 */
	RXADDR->rxcs = FILL | GO;
	/*
	 * get core address and byte count
	 */
	cp = rxstat.coreaddr;
	wc = ((rxstat.bytect <= -128)? 128 : -rxstat.bytect);
	/*
	 * save and set segmentation register.
	 */
	a = KISA->r[5];
	d = KISD->r[5];
	spl7();
	KISA->r[5] = rxstat.coreblk;
	KISD->r[5] = 01006;
	/*
	 * move wc bytes from the in-core buffer to
	 *   the device buffer
	 */
	for(i=wc;  i>0; --i) {
		WAITTR
		RXADDR->rxdb = *cp++;
	}
	/*
	 * sluff excess bytes
	 */
	for(i=128-wc; i>0; --i) {
		WAITTR
		RXADDR->rxdb = 0;
	}
	KISA->r[5] = a;
	KISD->r[5] = d;
	spl5();
	rxstat.coreblk =+ 2;
}
#endif

rxtimeout(dummy)
{
	static int prevreq;
	register struct buf *bp;

	bp = rxtab.d_actf;
	/*
	 * if the queue isn't empty and the current request number is the
	 * same as last time, abort the buffer and restart i/o.
	 */
	if(bp) {
		if(prevreq == rxstat.reqnum) {
			prdev("Floppy timeout", bp->b_dev);
			rxabtbuf();
			rxstart();
		}
		prevreq = rxstat.reqnum;
		timeout(rxtimeout, 0, TTIME);
	} else {
		/*
		 * if queue is empty, just quit and rxstrategy will
		 * restart us.
		 */
		rxstat.toutact = 0;
	}
}

rxabtbuf()
{
	register struct buf *bp;

	/*
	 * abort the current buffer with an error and unlink it.
	 */
	bp = rxtab.d_actf;
	bp->b_flags =| B_ERROR;
	rxtab.d_actf = bp->av_forw;
	rxtab.d_errcnt = rxtab.d_active = 0;
	iodone(bp);
}

#ifdef DEBUG
rxdebug() {
	register struct buf *bp;

	spl5();
	printf("Debug:  &rxtab=%o, &rxstat=%o\n", &rxtab, &rxstat);
	printf(" rxstat:  fminor=%l, bytect=%l, sec=%l\n",
		rxstat.fminor, -rxstat.bytect, rxstat.sector);
	printf("   reqnum=%l\n", rxstat.reqnum);
	printf(" rxtab:  d_active=%l, buffers:\n", rxtab.d_active);
	for(bp=rxtab.d_actf; bp; bp=bp->av_forw)
		printf(" dev=%l/%l, blkno=%l, wcnt=%l, flags=%o.\n", bp->b_dev.d_major,
			bp->b_dev.d_minor, bp->b_blkno, -bp->b_wcount, bp->b_flags);
	putchar('\n');
}
#endif
