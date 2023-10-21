#include "../param.h"
#include "../cf.h"
cf_max(abp)			/* return maximum of a bnode's free areas
				   and those of its children */
{
	register struct bnode *bp;
	register struct key *k, *lim;
	int max, size;

	bp = abp;
	k = bp->keys;
	lim = &(bp->keys[bp->nfarea]);
	max = k->max;
	while (k<lim) {
		if((size = k->last - k->base + 1) > max)
			max = size;
		if((++k)->max > max)
			max = k->max;
	}
	return(max);
}

cf_getmax()			/* update the max field of abp's ancestors */
{
	register struct bnode *bp;
	register struct buf *bufp;
	register struct key *kf;
	int i;

	for(i = cf_depth; i >= 0; --i) {
		bufp = cf_buf[i];
		if(bufp != NULL) {
			bp = bnodes[i];
			if((kf = bp->father) != NULL)
				kf->max = cf_max(bp);
			bawrite(bufp);
		}
		cf_buf[i] = bnodes[i] = NULL;
	}
}
