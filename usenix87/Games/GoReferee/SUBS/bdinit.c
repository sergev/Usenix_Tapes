#include	"../godef.h"
/*
**      Initialize board & display
*/
bdinit(bp)
struct	bdstr	*bp;
{
	register int i, j;

	/* fill entire board with EMPTY spots */
	for (i = BAREA; --i >= 0; ) {
	    bp->b_spot[i].s_occ = EMPTY;
	    bp->b_spot[i].s_flg = 0;
	}
	/* mark the borders as such */
	for (i = BSZ2; --i >= 0; ) {
	    j = i + BSZ2 * 0;
	    bp->b_spot[j].s_occ = BORDER;
	    j = i + BSZ2 * (BSZ2 - 1);
	    bp->b_spot[j].s_occ = BORDER;
	    j = 0 + BSZ2 * i;
	    bp->b_spot[j].s_occ = BORDER;
	    j = (BSZ2 - 1) + BSZ2 * i;
	    bp->b_spot[j].s_occ = BORDER;
	}
}
