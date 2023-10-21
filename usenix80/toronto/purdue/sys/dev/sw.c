#
/*
 * sw.c - pseudo swap device driver
 * The /dev/sw is a "logical" swap area to the system.  This
 * driver just takes requests and maps them to the actual
 * physical swap device.  See low.s for definations of
 * pswapdev, sswapdev, pswplo, sswplo, paddrx, ptimx.
 * -ghg 09/17/77.
 */

#include "../param.h"
#include "../user.h"
#include "../systm.h"
#include "../conf.h"
#include "../buf.h"

struct devtab swtab;
swstrategy(abp)
struct buf *abp;
{
	register struct buf *bp;
	register swdev;

	bp = abp;
	if(bp->b_blkno <= paddrx) {
		swdev = pswapdev;	/* primary swap (fast) dev */
		bp->b_blkno = bp->b_blkno + pswplo;
	}
	else {
		swdev = sswapdev;	/* secondary (slow) swap */
		bp->b_blkno = bp->b_blkno - paddrx + sswaplo;
	}
	bp->b_dev = swdev;
	(*bdevsw[swdev>>8].d_strategy)(bp);
}
