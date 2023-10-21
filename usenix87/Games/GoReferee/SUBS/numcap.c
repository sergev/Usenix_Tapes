#include	"../godef.h"

extern	int     dd[];
extern	struct	bdstr	b;

numcap(us, mv)			/* How many does a move at mv capture? */
{
	register int caps, them, d, ns, i;
	int capture();

	if (mv == PASS || mv == RESIGN)
	    return(0);
	caps = 0;
	them = (BLACK + WHITE) - us;
	for (d = 4; --d >= 0; ) {
	    ns = mv + dd[d];
	    if (b.b_spot[ns].s_occ == them && liberties(ns, &b) == 1)
		caps += gsize(ns, &b);
	}
	return(caps);
}
