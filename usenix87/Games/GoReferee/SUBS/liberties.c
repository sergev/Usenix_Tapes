#include	"../godef.h"

/*
**	LIBERTIES -- Return number of liberties in group containing
**		specified spot.
*/

extern	char	fmtbuf[];
extern	int     dd[];
extern	struct	bdstr	b;
extern	struct	bdstr	*grpopbp;
#ifdef	DEBUG
extern	char    *stoc();
#endif	DEBUG

liberties(sn, bp)
struct	bdstr	*bp;
{
	int libcheck();

#ifdef	DEBUG
	sprintf(fmtbuf, "liberties(%s)", stoc(sn));
	log(fmtbuf); read(0, fmtbuf, 10);
#endif DEBUG
	clearflg(LIBFLG, bp);
	grpopbp = bp;
	return(grpop(sn, libcheck, LIBFLG));
}

libcheck(sn)
{
	register int ns, d, n;

	n = 0;
	for (d = 4; --d >= 0; ) {
	    ns = sn + dd[d];
	    if (grpopbp->b_spot[ns].s_occ == EMPTY
	     && (grpopbp->b_spot[ns].s_flg & LIBFLG) == 0) {
		n++;
	        grpopbp->b_spot[ns].s_flg |= LIBFLG;
	    }
	}
#ifdef	DEBUG
	sprintf(fmtbuf, "libcheck(%s) returns %d", stoc(sn), n);
	log(fmtbuf); read(0, fmtbuf, 10);
#endif DEBUG
	return(n);
}
