182c
			if(t_openf[unit]>0 && bp != &rtmbuf && (TMADDR->tmer&EOF)==0)
.
171c
		deverror(bp, TMADDR->tmer, 0);
.
91a
	if (bp->b_flags&B_PHYS)
		mapalloc(bp);
.
5,8d
2a
 * tm.c - TM tape driver
 *
 *	modified by D A Gwyn on 25-Apr-1980:
 *	1) installed Ken's mods.
.
w
q
