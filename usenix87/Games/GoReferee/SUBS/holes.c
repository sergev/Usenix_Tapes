#include	"../godef.h"

/*
**	holes(bp, gbp, hbp, mgs) based on data in bp and gbp generate holes and
**		return hole indices in hbp.  mgs is minimum group size.
*/

extern	char	fmtbuf[];
extern	int     dd[];
extern	struct	bdstr	*grpopbp, x;
#ifdef	DEBUG
extern	char    *stoc();
#endif	DEBUG

int	numhls	= 0;		/* how many holes exist */
extern	int	numgrps;	/* how many groups exist (groups.c) */
static	char gr[MAXGRPS];

extern	struct	gstr	g[];	/* we use the group info from groups() */
struct	hstr	h[MAXGRPS];	/* here's where we leave the hole info */
struct	bdstr	*hnbp, *gnbp;	/* so hladd() can use them */

holes(bp, gbp, hbp, mgs)
struct	bdstr	*bp, *gbp, *hbp;
{
	register int i, j, n, q;
	int hladd();
	struct spotstr *sp;

#ifdef	DEBUG
	sprintf(fmtbuf, "holes(0x%x, 0x%x)", bp, hbp);
	log(fmtbuf); read(0, fmtbuf, 10);
#endif DEBUG
	numhls = 0;
	clearflg(GRPFLG, bp);		/* bp must not == hbp */
	for (i = BAREA; --i > 0; )	/* in hbp use flg to hold hole num */
	    hbp->b_spot[i].s_flg = 255;
	grpopbp = bp;			/* global used by grpop() & hladd() */
	gnbp = gbp;			/* global used by hladd() */
	hnbp = hbp;			/* global used by hladd() */
	for (i = BAREA; --i > 0; ) {
	    sp = &bp->b_spot[i];
	    if (sp->s_occ == EMPTY && (sp->s_flg & GRPFLG) == 0) {
		for (j = numgrps; --j >= 0; gr[j] = 0);
		h[numhls].h_spot = i;
		h[numhls].h_size = grpop(i, hladd, GRPFLG);
		for (n = j = q = 0; j < numgrps; j++) {
		    if (gr[j] && g[j].g_size > mgs) {
			if (n < MAXLIBS)
			    h[numhls].h_g[n] = j;
			q |= g[j].g_who;
			n++;
		    }
		}
		h[numhls].h_grps = n;	/* how many surrounding groups */
		h[numhls].h_who = q;	/* who surrounds it */
		numhls++;
		if (numhls >= MAXGRPS)
		    break;		/* should never happen, but */
	    }
	}
	return(numhls);
}

hladd(sn)
{
	register int d, n;

	hnbp->b_spot[sn].s_flg = numhls;
	for (d = 4; --d >= 0; ) {
	    n = gnbp->b_spot[sn + dd[d]].s_flg;
	    if (n < numgrps)
		gr[n] = 1;
	}
	return(1);
}
