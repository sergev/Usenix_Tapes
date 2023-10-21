#include	"../godef.h"

extern	char	*color[];
extern	char	fmtbuf[];
extern	struct  plyrstr	p[];
extern	int     dd[];
extern	struct	bdstr	b;

char	whyillegal	= 0;

extern	char    *stoc();

/* check the spot sn for legality */
legality(us, sn)
{
	register int i, them, d, ns;
	char buf[80];

#ifdef	DEBUG
	sprintf(fmtbuf, "legality(%s, %s)?", color[us], stoc(sn));
	log(fmtbuf); read(0, fmtbuf, 10);
#endif DEBUG
	whyillegal = 0;
	if (sn < 0 || sn >= BAREA
	 || b.b_spot[sn].s_occ != EMPTY) {
	    whyillegal = ILL_NOTEMPTY;
	    return(ILLEGAL);
	}
	if (p[us].p_ko == sn && numcap(us, sn) == 1) {
	    whyillegal = ILL_INTOKO;
	    return(ILLEGAL);
	}
	b.b_spot[sn].s_occ = us;
	them = (BLACK + WHITE) - us;
	if (liberties(sn, &b) == 0) {	/* need to capture to do this */
#ifdef	DEBUG
	    sprintf(fmtbuf, "in legality() liberties(%s, &b) is 0!", stoc(sn));
	    log(fmtbuf); read(0, fmtbuf, 10);
#endif DEBUG
	    for (d = 4; --d >= 0; ) {
		ns = sn + dd[d];
		if (b.b_spot[ns].s_occ == them && liberties(ns, &b) == 0)
		    break;		/* ok, we capture >= 1 of them */
	    }
	    if (d < 0)
		whyillegal = ILL_SUICIDE;
	}
#ifdef	DEBUG
	sprintf(fmtbuf, "legality(%s, %s) sets whyillegal to %d",
	 color[us], stoc(sn), whyillegal);
	log(fmtbuf); read(0, fmtbuf, 10);
#endif DEBUG
	b.b_spot[sn].s_occ = EMPTY;
	if (whyillegal != 0)
	    return(ILLEGAL);
	return(LEGAL);
}
