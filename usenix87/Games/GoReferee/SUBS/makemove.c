#include	"../godef.h"

extern	char	fmtbuf[128];
extern	char	whyillegal;		/* from legality.c */
extern	int     dd[];
extern	struct  plyrstr	p[3];
extern	struct	bdstr	b;
extern	struct	bdstr	*grpopbp;
extern	char    *stoc();

makemove(us, mv) /* Carry out player's move & update board */
{
	register int caps, them, d, ns, i;
	int capture();

	if (mv == PASS || mv == RESIGN)
	    return(LEGAL);
	if (legality(us, mv) == ILLEGAL) {
	    if (whyillegal == ILL_INTOKO)
		log("ko violation!");
	    else
		return(ILLEGAL);
	}
	b.b_spot[mv].s_occ = us;
	caps = 0;
	them = (BLACK + WHITE) - us;
	grpopbp = &b;
	for (d = 4; --d >= 0; ) {
	    ns = mv + dd[d];
	    if (b.b_spot[ns].s_occ == them && liberties(ns, &b) == 0) {
#ifdef	DEBUG
		sprintf(fmtbuf, "liberties(%s, &b) = 0", stoc(ns));
		log(fmtbuf); read(0, fmtbuf, 10);
#endif DEBUG
		clearflg(GRPFLG, &b);
		i = grpop(ns, capture, GRPFLG);
		caps += i;
		if (i == 1)
		    p[them].p_ko = ns;
	    }
	}
	if (caps != 1)
	    p[them].p_ko = 0;
	p[us].p_captures += caps;
	return(LEGAL);
}

capture(sn)	/* called by grpop() */
{
	grpopbp->b_spot[sn].s_occ = EMPTY;
	return(1);
}
