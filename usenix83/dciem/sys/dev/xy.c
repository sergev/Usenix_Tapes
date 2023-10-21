#include "../h/mascot.h"
/*
 * Disc driver for XYLOGICS controller
 * with 80mb or 300mb CDC storage modules.
 * For UNIX Version 7.
 * Martin Tuori, DCIEM, Toronto, Canada. May 20, 1981.
 */

#include "../h/local.h"
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/buf.h"
#include "../h/conf.h"
#include "../h/dir.h"
#include "../h/user.h"


struct	device
{
	int	xycsr;		/* control and status register */
	int	xydrv_trk_sec;	/* drive, head, sector register */
	caddr_t	xyba;		/* unibus address register */
	int	xywc;		/* word count register */
	int	xycyl;		/* cylinder address */
	int	xyds;		/* drive status register */
	int	xyer;		/* error register */
};

#define XYADDR ((struct device *)0164000)
#define	NXY	2

/* define XY80MEG 1     here, if you want 80 MB disc support; else you get 300MB */
#ifdef XY80MEG
#define NHEADS	5	/* 80 Megabyte has 5 heads */
#endif
#ifndef XY80MEG
#define NHEADS	19	/* 300 Megabyte has 19 heads */
#endif
#define NSECT	32	/* sectors per track */

struct	size
{
	daddr_t	nblocks;
	int	cyloff;
} xy_sizes[NXY<<3] =
{
	/* drive 0 */
	NSECT*NHEADS*23,	0,	/* swap = 13984 blocks.	0 */
	NSECT*NHEADS*400L,	23,	/* root.		1 */
	NSECT*NHEADS*400L,	423,	/* ?			2 */
	NSECT*NHEADS*0L,	0,	/* unused		3 */
	NSECT*NHEADS*0L,	0,	/* unused		4 */
	NSECT*NHEADS*0L,	0,	/* unused		5 */
	NSECT*NHEADS*0L,	0,	/* unused		6 */
	NSECT*NHEADS*0L,	0,	/* unused		7 */

	/* drive 1 */
	NSECT*NHEADS*23L,	0,	/* tmp			8 */
	NSECT*NHEADS*400L,	23,	/* user			9 */
	NSECT*NHEADS*400L,	423,	/* ?			10 */
	NSECT*NHEADS*0L,	0,	/* unused		11 */
	NSECT*NHEADS*0L,	0,	/* unused		12 */
	NSECT*NHEADS*0L,	0,	/* unused		13 */
	NSECT*NHEADS*0L,	0,	/* unused		14 */
	NSECT*NHEADS*0L,	0,	/* unused		15 */
};

			/* Drive Commands */
#define	GO	01
#define SYSCLEAR 0
#define SEEK	1
#define READ	04
#define WRITE	06
#define DRVCLEAR	022
#define RECAL	024
#define IENABLE	0100
				/* error codes and conditions */
#define XYREADY	0200
#define XYSEEKDONE 0200
#define XYERR	0100000		/* OR of all error bits */
#define HARD_SEEK	02000
#define DRIVE_FAULT	04000

#define XYMAXRETRY	12
int xyservo[XYMAXRETRY] = {
/* servo adjust for marginal data recovery -- see hw manual */
	00000,
	00000,
	00000,
	00000,
	00400,
	01000,
	02000,
	04000,
	02400,
	03000,
	04400,
	05000,
};

struct	buf	xytab;
struct	buf	rxybuf;

#define	b_cylin	b_resid

xystrategy(bp)
	register struct buf *bp;
{
	register struct buf *dp;
	register unsigned mindev;
	long sz;

	mindev = minor(bp->b_dev);

	sz = bp->b_bcount;
	sz = (sz+511)>>9;

	if(((mindev>>3) >= NXY) ||
	    bp->b_blkno < 0 ||
	    (bp->b_blkno + sz) > xy_sizes[mindev].nblocks){

		bp->b_flags |= B_ERROR;

		iodone(bp);
		printf("xystrategy: error.\n");
		deverror(bp, bp->b_blkno+sz, xy_sizes[mindev].nblocks);
		return;
	}
	bp->b_cylin = bp->b_blkno/(NSECT*NHEADS) + xy_sizes[mindev].cyloff;
	dp= &xytab;
	spl5();
	disksort(dp, bp);
	if (dp->b_active == 0) {
		xystart();
	}
	spl0();
}

xystart()
{
	register struct buf *bp;
	register mindev;
	daddr_t bn;
	int dn, sn, hn;
	unsigned int com;

	if ((bp = xytab.b_actf) == NULL)
		return;
	xytab.b_active++;
	mindev = minor(bp->b_dev);
	dn = mindev >> 3;
	bn = bp->b_blkno;

	hn= (bn/NSECT)%NHEADS;
	sn = bn%NSECT;

	XYADDR->xycyl= bp->b_cylin;
	XYADDR->xywc= -(bp->b_bcount>>1);
	XYADDR->xyba= bp->b_un.b_addr;
	XYADDR->xydrv_trk_sec= (dn<<12) | (hn<<7) | sn;
	com= GO | IENABLE | ((bp->b_xmem & 03) << 12) |
		((bp->b_flags & B_READ) ? READ : WRITE) |
		xyservo[xytab.b_errcnt];
	XYADDR->xycsr= com;
}

xyintr()
{
	register struct buf *bp;
	register int ctr;

	if (xytab.b_active == 0)
		return;
	bp = xytab.b_actf;
	xytab.b_active = 0;
	if(XYADDR->xycsr & XYERR){	/* error bit */
		/* do deverror only on first and last retries */
		if(!xytab.b_errcnt) deverror(bp, XYADDR->xyer, XYADDR->xyds);
		if(++xytab.b_errcnt<XYMAXRETRY){
			XYADDR->xycsr= SYSCLEAR|GO;
			ctr=0;
			while(!(XYADDR->xycsr &XYREADY) && --ctr)
				;
/*			if(!ctr) printf("XY controller didn't return from SYSCLEAR\n");*/
			if(XYADDR->xyer & HARD_SEEK) {
				XYADDR->xycsr = RECAL | GO;
				ctr = 0;
				while (!(XYADDR->xyds & XYSEEKDONE) && --ctr)
					;
				if(!ctr) printf("XY drive didn't return from RECAL\n");
			}
			XYADDR->xycsr= DRVCLEAR | GO;
/* At one point in history, we found it necessary to stop with NO retries,
 * as the disc seemed only to garbage itself worse. Errors are very rare
 * when the disc works right (99.5% of the time), but when it's wrong,
 * it's out to lunch.
 */
		}
		if (xytab.b_errcnt < XYMAXRETRY) {
			xystart();
			return;
		}
		printf("XY maxretry, csr:%o, dsr:%o, esr:%o, ba:%o, on /dev/xy%d, blk %ld\n",
			XYADDR->xycsr,XYADDR->xyds,XYADDR->xyer,XYADDR->xyba,
			minor(bp->b_dev), bp->b_blkno);
		bp->b_flags |= B_ERROR;
/* To really short circuit any further disc activity, insert: 
 *		panic("XY disc just died");
 * See note above.
 */
	}
	xytab.b_errcnt = 0;
	xytab.b_actf = bp->av_forw;
	bp->b_resid = XYADDR->xywc;
	iodone(bp);
	xystart();
}

xyread(dev)
{

	physio(xystrategy, &rxybuf, dev, B_READ);
}

xywrite(dev)
{

	physio(xystrategy, &rxybuf, dev, B_WRITE);
}
