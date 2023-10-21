#include	"../godef.h"

/*
**	groupsort(gbp) rearrange groups so biggest comes first
*/

#ifdef	DEBUG
extern	char	fmtbuf[];
extern	char    *stoc();
#endif	DEBUG

extern	int	numgrps;		/* how many groups exist */

extern	struct	gstr	g[MAXGRPS];

groupsort(gbp)
struct	bdstr	*gbp;
{
	register int i, j, k;
	struct spotstr *sp;
	struct gstr gg;

#ifdef	DEBUG
	sprintf(fmtbuf, "groupsort(0x%x)", gbp);
	log(fmtbuf); read(0, fmtbuf, 10);
#endif DEBUG
	for (i = numgrps; --i > 0; ) {
	    k = 0;
	    for (j = 1; j <= i; j++) {
		if (g[j].g_size < g[k].g_size)
		    k = j;
	    }
	    if (k != i) {
		gg = g[i]; g[i] = g[k]; g[k] = gg;
		for (sp = &gbp->b_spot[BAREA]; --sp > gbp->b_spot; ) {
		    if (sp->s_flg == i)
			sp->s_flg = k;
		    else if (sp->s_flg == k)
			sp->s_flg = i;
		}
	    }
	}
}
