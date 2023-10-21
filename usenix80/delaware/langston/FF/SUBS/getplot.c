#include    "../ffdef.h"
getplot(pp)                        /* read the next available plot into pp */
struct  plot    *pp;
{
	register int strt, i;

	getmisc(SAFE);
	if (misc.m_lstplt <= 0)
	    pp->p_xy = NO_PLOT_LEFT;
	else {
	    --(misc.m_lstplt);
	    seek(mfh, sizeof misc + 2 * misc.m_lstplt, 0);
	    read(mfh, pp, sizeof *pp);
	    putmisc();
	}
}
