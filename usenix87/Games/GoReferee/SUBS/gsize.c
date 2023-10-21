#include	"../godef.h"

/*
**	GSIZE -- Return size of group containing specified spot.
*/

extern	char	fmtbuf[];
extern	struct	bdstr	*grpopbp;
#ifdef	DEBUG
extern	char    *stoc();
#endif	DEBUG

gsize(sn, bp)
struct	bdstr	*bp;
{
	int one();

#ifdef	DEBUG
	sprintf(fmtbuf, "gsize(%s)", stoc(sn));
	log(fmtbuf); read(0, fmtbuf, 10);
#endif DEBUG
	clearflg(GRPFLG, bp);
	grpopbp = bp;
	return(grpop(sn, one, GRPFLG));
}

one()
{
	return(1);
}
