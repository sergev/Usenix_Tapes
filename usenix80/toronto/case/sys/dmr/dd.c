#
/*
 *	DD disk driver
 *
 *	Diablo model 40 disk
 *	System Industries controller
 *
 *	Bill Shannon - CWRU   Nov 78
 *
 *	Should handle 4 drives per controller
 *	but has only been tested with one.
 *
 *	NOTE: The model 3047 controller will not cross 32K
 *	memory boundaries on data transfers (the xmem bits
 *	are not incremented), where the model 3057 will.
 *	There are no good ways to fix this, only kludges:
 *
 *		1. Modify main.c to not add segments that
 *		   start on 32K word boundaries to the free
 *		   list.  This prevents anything from crossing
 *		   the boundaries but leaves holes in the memory.
 *
 *		2. Modify the controller to increment the xmem
 *		   bits.  (Not a lot of fun but we've done it.)
 */

/*
 *	name	minor	port	size	fixed/removable
 *	----	-----	----	----	---------------
 *	rd0	  0	 0	RDSIZE	fixed
 *	rd1	  1	 0	RDSIZE	removable
 *	rd2	  2	 1	RDSIZE	fixed
 *	rd3	  3	 1	RDSIZE	removable
 *	 .	  .	 .	  .	  .
 *	rd7	  7	 3	RDSIZE	removable
 *	dd0	  8	 0	DDSIZE	fixed
 *	dd1	  9	 0	DDSIZE	fixed
 *	dd2	 10	 0	DDSIZE	fixed
 *	dd3	 11	 0	DDSIZE	fixed
 *	dd4	 12	 0	DDSIZE	removable
 *	dd5	 13	 0	DDSIZE	removable
 *	dd6	 14	 0	DDSIZE	removable
 *	dd7	 15	 0	DDSIZE	removable
 *	dd8	 16	 1	DDSIZE	fixed
 *	 .	  .	 .	  .	  .
 */

#include "../param.h"
#include "../buf.h"
#include "../conf.h"
#include "../user.h"

#define NDD	1		/* number of drives */
#define DDADDR 0166540
#define BARR   0166544
#define BARW   0166546
#define NDDBLK (1632*12)	/* 1632 tracks 12 blocks per track */
#define RDSIZE (NDDBLK/2)	/* blocks per platter */
#define DDSIZE (NDDBLK/8)	/* for accessing our RT-11 disks */

#define IENABLE 0100 
#define DDBUSY  02
#define DDDONE  01

struct {
	int ddcer;
	int ddcar;
	int ddbarr;
	int ddbarw;
};

struct devtab	ddtab;
struct buf	rddbuf;
int	ddport;

ddstrategy(abp)
struct buf *abp;
{
	register struct buf *bp;
	register int s, t;

	bp = abp;
	if (bp->b_dev.d_minor < 8) {
		s = RDSIZE;
		t = bp->b_dev.d_minor > NDD*2;
	} else {
		s = DDSIZE;
		t = ((bp->b_dev.d_minor - 8) >> 3) > NDD*2;
	}
	if (t || bp->b_blkno >= s) {
		if (bp->b_flags&B_READ)
			bp->b_resid = bp->b_wcount;
		else {
			bp->b_flags =| B_ERROR;
			bp->b_error = ENXIO;
		}
		iodone(bp);
		return;
	}
	bp->av_forw = 0;
	spl5();
	if (ddtab.d_actf == 0)
		ddtab.d_actf = bp;
	else
		ddtab.d_actl->av_forw = bp;
	ddtab.d_actl = bp;
	if (ddtab.d_active == 0)
		ddstart();
	spl0();
}

ddaddr(abp)
struct buf *abp;
{
	register struct buf *bp;
	register int d, u;

	bp = abp;
	d = bp->b_dev.d_minor;
	if (d < 8) {
		ddport = d >> 1;
		u = (d&1) << 15;
	} else {
		ddport = (d-8) >> 3;
		u = ((d&4) << 13) + (d&3)*DDSIZE;
	}
	return(bp->b_blkno + u);
}

ddstart()
{
	register struct buf *bp;

	if ((bp = ddtab.d_actf) == 0)
		return;
	ddtab.d_active++;
	if (DDADDR->ddcer & DDBUSY)
		return;
	ddstrt(bp);
}

ddintr()
{
	register struct buf *bp;

	if (ddtab.d_active == 0)
		return;
	bp = ddtab.d_actf;
	ddtab.d_active = 0;
	if (DDADDR->ddcer < 0) {
		deverror(bp, DDADDR->ddcer, bp->b_wcount);
		DDADDR->ddcer = 0;
/*
		while ((DDADDR->ddcer & DDDONE) == 0);
*/
		if (++ddtab.d_errcnt <= 10) {
			ddstart();
			return;
		}
		bp->b_flags =| B_ERROR;
	}
	ddtab.d_errcnt = 0;
	ddtab.d_actf = bp->av_forw;
	iodone(bp);
	ddstart();
}

ddstrt(abp)
struct buf *abp;
{
	register struct buf *bp;
	register int *barptr;
	register int bar;
	static struct {
		int wcount;
		char *addr;
	} dmablk;

	bp = abp;
	bar = ddaddr(bp);	/* also sets ddport */
	barptr = ((bp->b_flags & B_READ ) ? BARR:BARW);
	dmablk.wcount = (-bp->b_wcount);
	dmablk.addr = bp->b_addr;
	DDADDR->ddcer = 0;	/* make sure xmem bits are zero for dmablk */
	DDADDR->ddcar = &dmablk;
	DDADDR->ddcer = IENABLE | ((bp->b_xmem & 03) << 7) | ddport;
	*barptr = bar;
}

ddread(dev)
{
	physio(ddstrategy, &rddbuf, dev, B_READ, dev.d_minor<8 ? RDSIZE : DDSIZE);
}

ddwrite(dev)
{
	physio(ddstrategy, &rddbuf, dev, B_WRITE, dev.d_minor<8 ? RDSIZE : DDSIZE);
}
