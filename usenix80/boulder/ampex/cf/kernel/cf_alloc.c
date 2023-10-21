#include "../param.h"
#include "../user.h"
#include "../cf.h"
cf_getfree(nchunks)		/* return chunk number on contiguous device
				   of an area of size nchunks, 
				   and update free list */
{
	register struct bnode *bp;
	register struct key *k, *maxson;
	int size, ans, depth;

	for(depth = 1; bp = bnodes[depth]; depth++) {		/* start search at root */
		maxson = NULL;
		if(u.u_error)
			return;
		k = bp->keys;
		if(k->max >= nchunks)
			maxson = k;
		while(k < &(bp->keys[bp->nfarea])) {
			if((size = k->last - k->base) >= nchunks) {	/* found */
				ans = k->base;
				cf_update(depth, k, -nchunks, 0);
				return(ans);
			}
			if((++k)->max >= nchunks)
				maxson = k;
		}
		if(maxson == NULL)
			u.u_error = E2BIG;
		else
			getbuf(maxson->lson, maxson, 0);
	}
}

cf_update(depth, ak, sizel, sizer)	/* increase the left and right edges
					   of a free area */
{
	register struct bnode *bp;
	register struct key *k, *k1;
	int rem, bound, newbase;

	bp = bnodes[depth];
	bound = cf_sb.bound;
	k1 = k = ak;
	k->base =- sizel;
	k->last =+ sizer;
	combine(depth, k, k+1, bound);
	if(combine(depth, k-1, k, bound) || (rem = k->base%bound) == 0
	    || k->base/bound == k->last/bound)
		return;
	newbase = k->base;
	k->base =+ (bound - rem);
	cf_newk(depth, k, newbase, k->base);
}

combine(depth, akl, akr, bound)
{
	register struct bnode *bp;
	register struct key *kl, *kr;
	int rem;

	bp = bnodes[depth];
	kl = akl;
	kr = akr;
	if(kl < bp->keys || kr >= &(bp->keys[bp->nfarea])
	    || kl->last + 1 < kr->base)
		return(0);
	kl->last = kr->last;
	if((rem = kl->base % bound) == 0 || kl->base/bound == kl->last/bound)
		cf_delete(depth, kr);
	else {
		kr->base = kl->base + bound - rem;
		kl->base = kr->base - 1;
	}
	return(1);
}

cf_delete(depth, ak)
{
	register struct bnode *bp;
	register struct key *k, *k1;

	bp = bnodes[depth];
	k = ak;
	while(k->rmax > 0) {	/* at a non-leaf node, so
				   promote first larger son */
		getbuf(k->rson, k+1, 0);
		bp = bnodes[++depth];
		k1 = bp->keys;
		bcopy(&(k1->base), &(k->base), FSIZ);
		k = k1;			/* "recursive call" */
	}
	if(bp->nfarea > 0) {
		k = &(bp->keys[--(bp->nfarea)]);
		bcopy(k+1, k, (k1-k)*KSIZ+PSIZ);
	}
}
