#ifndef lint
static char rcsid[] = "$Header: bursts.c,v 1.2 85/07/08 17:22:19 matt Exp $";
#endif
/*
 *
 * search
 *
 * multi-player and multi-system search and destroy.
 *
 * Original by Greg Ordy	1979
 * Rewrite by Sam Leffler	1981
 * Socket code by Dave Pare	1983
 * Ported & improved
 *      by Matt Crawford	1985
 *
 * routines to manipulate shrapnel
 *
 * Copyright (c) 1979
 *
 */


#include "defines.h"
#include "structs.h"

/*
 * Create a burst of shrapnel upon death of a player.
 */
void makeburst(bx, by)
char bx, by;
{
	extern	t_burst bursts[NBURST];
	register t_burst *bp;
	register t_burst *last;

	last = &bursts[NBURST];
	for (bp = bursts; bp < last; bp++) {
		if (bp->cbactive != 0)
			continue;
		bp->cbactive++;
		bp->cbx = bx;
		bp->cby = by;
		bp->shrap[0][0] = bx;
		bp->shrap[0][1] = by;
		bp->cbcnt = 100;
		return;
	}
}

/*
 * Update shrapnel position and display
 */
void moveburst() {
	extern	int nplayer;
	extern	char xxi,
		yyi;
	extern	t_burst bursts[NBURST];
	void	getdir();
	register int j;
	register t_burst *bp,
		*last;
	register char	xi,
		yi,
		qrt;

	last = &bursts[NBURST];
	for (bp = bursts; bp < last; bp++) {
		if (bp->cbactive == 0)
			continue;
		if (bp->cbcnt == 100) {
			bp->cbcnt--;
			break;
		}
		if (bp->cbcnt == 98) {
			bp->cbcnt = 50;
			for (j=1; j != 4; j++) {
				qrt = (rand()>>4)%8;
				xi = bp->cbx;
				yi = bp->cby;
				switch (qrt) {
				case 0: 
					xi++;
					break;
				case 1: 
					xi++;
					yi--;
					break;
				case 2: 
					yi--;
					break;
				case 3: 
					xi--;
					yi--;
					break;
				case 4: 
					xi--;
					break;
				case 5: 
					xi--;
					yi++;
					break;
				case 6: 
					yi++;
					break;
				case 7: 
					xi++;
					yi++;
					break;
				}
				bp->shrap[j][0] = xi;
				bp->shrap[j][1] = yi;
				bp->shrapd[j] = qrt;
			}
			break;
		}
		if (bp->cbcnt > 48) {
			bp->cbcnt--;
			break;
		}
		if (bp->cbcnt == 48) {
			bp->cbcnt = (rand()>>4)%(16-nplayer);
			qrt = bp->shrapd[1];
			xi = bp->shrap[1][0];
			yi = bp->shrap[1][1];
			getdir(qrt, xi, yi);
			bp->shrap[0][0] = xxi;
			bp->shrap[0][1] = yyi;
			getdir(qrt, xi, yi);
			bp->shrap[4][0] = xxi;
			bp->shrap[4][1] = yyi;
			qrt = bp->shrapd[2];
			xi = bp->shrap[2][0];
			yi = bp->shrap[2][1];
			getdir(qrt, xi, yi);
			bp->shrap[5][0] = xxi;
			bp->shrap[5][1] = yyi;
			getdir(qrt, xi, yi);
			bp->shrap[6][0] = xxi;
			bp->shrap[6][1] = yyi;
			qrt = bp->shrapd[3];
			xi = bp->shrap[3][0];
			yi = bp->shrap[3][1];
			getdir(qrt, xi, yi);
			bp->shrap[7][0] = xxi;
			bp->shrap[7][1] = yyi;
			getdir(qrt, xi, yi);
			bp->shrap[8][0] = xxi;
			bp->shrap[8][1] = yyi;
			break;
		}
		bp->cbcnt--;
		if (bp->cbcnt == 3) {
			for (j = 1; j != 4; j++) {
				bp->shrap[j][0] = 0;
				bp->shrap[j][1] = 0;
			}
			bp->shrap[0][0] = 0;
			bp->shrap[0][1] = 0;
			break;
		}
		if (bp->cbcnt == 0) {
			for (j = 4; j != 9; j++) {
				bp->shrap[j][0] = 0;
				bp->shrap[j][1] = 0;
			}
			bp->cbactive = 0;
			break;
		}
	}
}

/*
 * Furnish random movement for shrapnel pieces
 */
static void getdir(dir, xi, yi)
char dir, xi, yi;
{
	extern	int rand();
	extern	char xxi,
		yyi;
	register char dx,
		dy;

	dx = ((rand()>>3)%3)-1;
	dy = ((rand()>>3)%3)-1;
	switch (dir) {
	case 0: 
		if (dx < 0)
			dx = 2;
		break;
	case 1: 
		if (dx < 0)
			dx = 2;
		if (dy > 0)
			dy = -2;
		break;
	case 2: 
		if (dy > 0)
			dy = -2;
		break;
	case 3: 
		if (dx > 0)
			dx = -2;
		if (dy > 0)
			dy = -2;
		break;
	case 4: 
		if (dx > 0)
			dx = -2;
		break;
	case 5: 
		if (dx > 0)
			dx = -2;
		if (dy < 0)
			dy = 2;
		break;
	case 6: 
		if (dy < 0)
			dy = 2;
		break;
	case 7: 
		if (dx < 0)
			dx = 2;
		if (dy < 0)
			dy = 2;
		break;
	}
	xxi = xi+dx;
	yyi = yi+dy;
}
