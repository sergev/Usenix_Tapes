#include	"../godef.h"

/*
**	groups(bp, gbp) based on data in bp generate groups and
**		return group indices in gbp;
*/

extern	char	fmtbuf[];
extern	int     dd[];
extern	struct	bdstr	*grpopbp;
#ifdef	DEBUG
extern	char    *stoc();
#endif	DEBUG

int	numgrps	= 0;		/* how many groups exist */

struct	gstr	g[MAXGRPS];
struct	bdstr	*gnbp;		/* so grpadd() can use it */

groups(bp, gbp)
struct	bdstr	*bp, *gbp;
{
	register int i;
	int grpadd();
	struct spotstr *sp;

#ifdef	DEBUG
	sprintf(fmtbuf, "groups(0x%x, 0x%x)", bp, gbp);
	log(fmtbuf); read(0, fmtbuf, 10);
#endif DEBUG
	numgrps = 0;
	clearflg(GRPFLG, bp);		/* bp must not == gbp */
	for (i = BAREA; --i > 0; )	/* in bgp use flg to hold group num */
	    gbp->b_spot[i].s_flg = 255;
	grpopbp = bp;			/* global used by grpop() & grpadd() */
	gnbp = gbp;			/* global used by grpadd() */
	for (i = BAREA; --i > 0; ) {
	    sp = &bp->b_spot[i];
	    if ((sp->s_occ == BLACK || sp->s_occ == WHITE)
	     && (sp->s_flg & GRPFLG) == 0) {
		clearflg(LIBFLG, bp);	/* get ready to count liberties */
		g[numgrps].g_who = sp->s_occ;
		g[numgrps].g_spot = i;
		g[numgrps].g_libs = 0;
		g[numgrps].g_size = grpop(i, grpadd, GRPFLG);
		numgrps++;
		if (numgrps >= MAXGRPS)
		    break;		/* should never happen, but */
	    }
	}
	return(numgrps);
}

grpadd(sn)
{
	register int ns, d, n;

	gnbp->b_spot[sn].s_flg = numgrps;
	n = g[numgrps].g_libs;
	for (d = 4; --d >= 0; ) {
	    ns = sn + dd[d];
	    if (grpopbp->b_spot[ns].s_occ == EMPTY
	     && (grpopbp->b_spot[ns].s_flg & LIBFLG) == 0) {
		if (n < MAXLIBS)
		    g[numgrps].g_l[n] = ns;
		n++;
	        grpopbp->b_spot[ns].s_flg = LIBFLG;
	    }
	}
	g[numgrps].g_libs = n;
#ifdef	DEBUG
	sprintf(fmtbuf, "libcheck(%s) returns %d", stoc(sn), n);
	log(fmtbuf); read(0, fmtbuf, 10);
#endif DEBUG
	return(1);
}
