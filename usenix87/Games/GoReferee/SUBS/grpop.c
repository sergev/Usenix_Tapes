#include	"../godef.h"

extern	char	fmtbuf[128];
extern	int     dd[4];
extern	char    *stoc();

struct	bdstr	*grpopbp;

/*
** Call op() once on each member of the connected group.  Use the board
** pointed at by global grpopbp; and the specified flag to avoid duplication.
** Return the sum of the returns from op()
*/

grpop(sn, op, flg)
int	(*op)();
{
	register int ns, d, n, same;

	same = grpopbp->b_spot[sn].s_occ;
	grpopbp->b_spot[sn].s_flg |= flg;
	n = 0;
#ifdef	DEBUG
	sprintf(fmtbuf, "grpop(%s, 0x%x, %d) same=%d",
	 stoc(sn), op, flg, same);
	log(fmtbuf); read(0, fmtbuf, 10);
#endif DEBUG
	for (d = 4; --d >= 0; ) {
	    ns = sn + dd[d];
#ifdef	DEBUG
	    sprintf(fmtbuf, "try %s; s_occ=%d, s_flg=%d",
	     stoc(ns), grpopbp->b_spot[ns].s_occ, grpopbp->b_spot[ns].s_flg);
	    log(fmtbuf); read(0, fmtbuf, 10);
#endif DEBUG
	    if (grpopbp->b_spot[ns].s_occ == same
	     && (grpopbp->b_spot[ns].s_flg & flg) == 0)
		n += grpop(ns, op, flg);
	}
	return(n + (*op)(sn));
}
