#
/*
 */

/*
 * RK disk driver
 *
 *	Modified - 04/06/78 by Brad Blasing, University of Minnesota.
 *	Overlapping seeks on multiple drives added.  This driver uses
 *	the feature in the RK controller where seeks may be initiated
 *	on any or all drives in addition to the one that is doing I/O.
 *	The scheme employed is:  before each buffer request is started,
 *	the queue of i/o yet to be started is searched and seek aheads
 *	are started on each drive which is not already busy doing a 
 *	seek.  Then the next i/o request is started.  Notice that the
 *	RK controller will not allow you to start seeks when i/o is
 *	occuring on another drive.  Therefore, you must start any seeks
 *	you want when the controller is idle and before you start the
 *	next i/o request.  That is why all the seek ahead is done in
 *	rkstart, which is called only when i/o is not active.
 *	An array of flags is kept (one flag wod per drive) to keep
 *	track of which drives have seeks active and seeks complete.
 *	Once a drive has had a seek initiated on it and the seek
 *	has completed, no more seeks will be issued to that drive
 *	until it has been used for i/o.  This keeps the drive at the
 *	right cylinder until its i/o buffer request gets honored.
 *
 *	This driver probably doesn't support the multiple drive per
 *	file system feature as described in RK(IV).
 */

/* #define DEBUG	for debugging only. */

#include "../param.h"
#include "../buf.h"
#include "../conf.h"
#include "../user.h"

#define	RKADDR	0177400
#define	NRK	4	/* set to num. of drives */
#define	NRKBLK	4872

#define	RESET	0
#define	GO	01
#define	DRESET	014
#define SEEK	010
#define	IENABLE	0100
#define	DRY	0200
#define	ARDY	0100
#define	WLO	020000
#define	CTLRDY	0200
#define SDONE	020000		/* seek done */

struct {
	int rkds;
	int rker;
	int rkcs;
	int rkwc;
	int rkba;
	int rkda;
};

struct	devtab	rktab;
struct	buf	rrkbuf;

char	rkstat[NRK];	/* Status of each drive */

/* Status flags for rkstat: */
#define S_SINIT	01	/* seek initiated or drive busy doing i/o */
#define S_SDONE	02	/* seek done */

rkstrategy(abp)
struct buf *abp;
{
	register struct buf *bp;
	register *qc, *ql;
	int d;

	bp = abp;
	if(bp->b_flags&B_PHYS)
		mapalloc(bp);
	d = bp->b_dev.d_minor-7;
	if(d <= 0)
		d = 1;
	if (bp->b_blkno >= NRKBLK*d) {
		bp->b_flags =| B_ERROR;
		iodone(bp);
		return;
	}
	bp->av_forw = 0;
	spl5();
	if (rktab.d_actf==0)
		rktab.d_actf = bp;
	else
		rktab.d_actl->av_forw = bp;
	rktab.d_actl = bp;
	if (rktab.d_active==0)
		rkstart();
	spl0();
}

rkseek(abp)
struct buf *abp;
{
	register struct buf *bp;
	register int d;

	bp = abp;
	d = bp->b_dev.d_minor & 07;
	/*
	 *  If drive active, abort.
	 */
	if(rkstat[d] & (S_SINIT | S_SDONE))
		return;
	/*
	 *  Start seek.
	 */
#ifdef DEBUG
	if(SW->integ == 0177400)
		printf("--seek started on buf %o\n", bp);
#endif
	rkstat[d] =| S_SINIT;
	RKADDR->rkda = rkaddr(bp);
	RKADDR->rkcs = SEEK | IENABLE | GO;
	/*
	 *  Wait til done.
	 */
	d = 0;
	while((RKADDR->rkcs & CTLRDY) == 0 && --d) ;
}

rkaddr(bp)
struct buf *bp;
{
	register struct buf *p;
	register int b;
	int d, m;

	p = bp;
	b = p->b_blkno;
	m = p->b_dev.d_minor - 7;
	if(m <= 0)
		d = p->b_dev.d_minor;
	else {
		d = lrem(b, m);
		b = ldiv(b, m);
	}
	return(d<<13 | (b/12)<<4 | b%12);
}

rkstart()
{
	register struct buf *bp,*bp2;
	register int d;

	if ((bp = rktab.d_actf) == 0)
		return;
	/*
	 *  Mark bp's drive as busy, abort if already busy.
	 */
	d = bp->b_dev.d_minor & 07;
	if((rkstat[d] & S_SINIT) && ((rkstat[d] & S_SDONE) == 0))
		return;
	rkstat[d] =| S_SINIT;
	/*
	 *  Scan queue and start seeks on all drives except the
	 *  the one bp refers to (its already marked busy and soon 
	 *  will be really busy).  Notice that rkstart is called
	 *  only when i/o is not active (rktab.d_active == 0).
	 */
#ifdef DEBUG
	if(SW->integ == 0177400) {
		printf("\nBuffers in queue:\n");
		printf("%o: drive=%d, bn=%d\n", bp, bp->b_dev.d_minor,
		bp->b_blkno);
	}
#endif
	bp2 = bp->av_forw;
	while(bp2) {
#ifdef DEBUG
		if(SW->integ == 0177400)
			printf("%o: drive=%d, bn=%d\n", bp2, bp2->b_dev.d_minor,
			bp2->b_blkno);
#endif
		rkseek(bp2);
		bp2 = bp2->av_forw;
	}
	/*
	 *  Start i/o
	 */
	rktab.d_active++;
	devstart(bp, &RKADDR->rkda, rkaddr(bp), 0);
}

rkintr()
{
	register struct buf *bp;
	register int cs,d;

	bp = rktab.d_actf;
	d = bp->b_dev.d_minor & 07;
	cs = RKADDR->rkcs;
	/*
	 *  Check for error.
	 */
	if (cs < 0) {		/* error bit */
		deverror(bp, RKADDR->rker, RKADDR->rkds);
		RKADDR->rkcs = RESET|GO;
		while((RKADDR->rkcs&CTLRDY) == 0) ;
		for(cs = NRK; cs != 0; ) rkstat[--cs] = 0;
		if (++rktab.d_errcnt <= 10) {
			rkstat[d] =& ~(S_SINIT | S_SDONE);
			rkstart();
			return;
		}
		bp->b_flags =| B_ERROR;
		goto out;
	}
	/*
	 *  Check to see if a seek completed.
	 */
	if(cs & SDONE) {
		/*
		 *  Flag seek as done.
		 */
		d = (RKADDR->rkds >> 13) & 07;
		rkstat[d] =| S_SDONE;
		if(rktab.d_active == 0)
			rkstart();
		return;
	}
	/*
	 *  Check for false entry (i.e. seek address ack interrupt
	 *  rather than a valid i/o done interrupt).  Check to see
	 *  if drive is unbusy physically but rktab.d_active is set.
	 *  If so, i/o must have finished and this is its interrupt.
	 */
	RKADDR->rkda = d << 13;		/* Set to status the right drive. */
	if(rktab.d_active == 0 || (RKADDR->rkds & ARDY) == 0)
		return;
	/*
	 *  Mark drive as unbusy.
	 */
out:
	rkstat[d] =& ~(S_SINIT | S_SDONE);
	rktab.d_active = 0;
	rktab.d_errcnt = 0;
	/*
	 *  Mark this buffer as done and unlink it from queue.
	 */
	rktab.d_actf = bp->av_forw;
	iodone(bp);
	rkstart();
}

rkread(dev)
{

	physio(rkstrategy, &rrkbuf, dev, B_READ);
}

rkwrite(dev)
{

	physio(rkstrategy, &rrkbuf, dev, B_WRITE);
}
