#include	"../godef.h"
/*
**      clear specified bit(s) in all flag words in specified board
**      psl 5/84
*/

clearflg(flg, bp)
struct	bdstr	*bp;
{
	register int i, mask;

	mask = ~flg;
	for (i = BAREA; --i > 0; )
	    bp->b_spot[i].s_flg &= mask;
}
