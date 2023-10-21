#include "../param.h"
#include "../user.h"
#include "../inode.h"
#include "../cf.h"
#include "../reg.h"

cf_trunc(ip)		/* called from "itrunc", only way that
			   1. a contiguous file is removed
			   2. a new area is inserted onto the free list 
			 */
register struct inode *ip;
{
	long l;
	int base, last;

	if(cf_start(ip->i_dev)) 
		cf_insert(btoc(ip->i_addr->i_cbase), btoc(ip->i_addr->i_clast));
	cf_relse();
	return(!u.u_error);
}

cf_insert(base, last)		/* insert a new free area */
{
	register struct bnode *bp;
	register struct key *k, *lim;
	int sizel, sizer, depth;

	sizel = sizer = 0;
	depth = 1;
	bp = bnodes[1];
    scanlr:			/* scan an array of keys, left to right */
	if(u.u_error)
		return;
	lim = &(bp->keys[bp->nfarea]);
	for (k = bp->keys; ; k++) {
		if(last+1 < k->base || k==lim) {	/* move down tree to 
						   left son if it exists */
			if(k->max == 0) {
				cf_newk(depth, k, base, last);
				return;
			}
			getbuf(k->lson, k, 0);
			bp = bnodes[depth+1];
			goto scanlr;
		}
		if(base-1 <= k->last) {		/* nodes can be overlapped */
			if(base < k->base)
				sizel = k->base - base;
			if(last > k->last)
				sizer = last - k->last;
			cf_update(depth, k, sizel, sizer);
			return;
		}
	}
}

btoc(n)			/* convert block number to chunk number */
long n;
{
	n =- cf_sb.cfb0;
	return(dpdiv(n, cf_sb.chunksiz));
}
