#include "../param.h"
#include "../cf.h"
#include "../user.h"
cf_newk(depth, oldk, base, last)	/* at leaf level, make a new key
				   to the left of oldk */
{
	register struct bnode *bp, *newbp;
	register struct key *k;
	struct key newk;
	int nkey, rs;

	makekey(&newk, NULL, 0, base, last);
	bp = bnodes[depth];
	for( ; ; ) {
		for(k= &(bp->keys[bp->nfarea++]); k>=oldk; --k)
			bcopy(k, k+1, KSIZ);
		bcopy(&newk, oldk, KSIZ);
		if(bp->nfarea < NKEY || u.u_error)
			return;
		nkey = (bp->nfarea)/2;
		k = &(bp->keys[nkey]);
		bcopy(k++, &newk, KSIZ);	/* middle key to be propagated up */
		newk.lson = getbuf(-1, bp->father, 0);
		newbp = bnodes[cf_depth];
		bcopy(bp->keys, newbp->keys, nkey*KSIZ+PSIZ);
		newbp->nfarea = nkey++;
		bp->nfarea =- nkey;
		bcopy(k, bp->keys, (bp->nfarea)*KSIZ+PSIZ);
		newk.max = cf_max(newbp);
		k = bp->father;
		putbuf();			/* write out new node */
		if(k == NULL) {			/* root is being split */
			rs = cf_sb.rootlbn;
			getbuf(-1, NULL, 1);
			k = bp->father = (bnodes[0])->keys;
			(bnodes[0])->nfarea = 0;
			k->lson = rs;
		}
		bp->father++;
		oldk = k;
		bp = bnodes[--depth];
	}
}

makekey(ak, lson, max, base, last)
{
	bcopy(&lson, ak, KSIZ);
}
