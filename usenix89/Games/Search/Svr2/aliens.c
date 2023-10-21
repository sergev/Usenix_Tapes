#ifndef lint
static char rcsid[] = "$Header: aliens.c,v 2.1 85/04/10 17:30:09 matt Stab $";
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
 * routines dealing with aliens
 *
 * Copyright (c) 1979
 *
 * $Log:	aliens.c,v $
 * Revision 2.1  85/04/10  17:30:09  matt
 * Major de-linting and minor restructuring.
 * 
 */


#include "defines.h"
#include "structs.h"

/*
 * Move an alien -- update displays
 */
void movealien(pa)
register t_alien *pa;
{
	extern	int rand();
	extern	void launch();
	void	phaser();
	int	psycho();
	register t_player *pl;
	char	dx,
		dy;

	pa->count--;
	/*
	 * If we're not on screen, just update internally
	 */
	if (pa->onscr == OFF || pa->whotoget == NOTHING) {
		if (pa->count == 0) {	/* shanker alarm clock */
			pa->count = (rand()%20)+100;
			pa->cix = (rand()%3)-1;
			pa->ciy = (rand()%3)-1;
			if (pa->cix == 0 && pa->ciy == 0)
				pa->ciy = -1;
		}
		pa->cx += pa->cix;
		pa->cy += pa->ciy;
		return;
	}
	pl = &pa->whotoget->u_player;
	pa->onscr = OFF;
	if (pa->count <= 0) {
		pa->count = 10;		/* reset alarm clock */
		dx = pl->curx - pa->cx;
		dy = pl->cury - pa->cy;
		/*
		 * Close enough to a player to shoot?
		 */
		if (dx < 9 && dx > -9 && dy < 5 && dy > -5) {
			pa->fx = pl->offx;
			pa->fy = pl->offy;
			if (pl->status.alive == FALSE) {
				pa->whotoget = 0;
				return;
			}
			if (dx == 0) {
				if (dy == 0) {
					phaser(pa, pl);
					return;
				}
				if (!psycho(pa))
					launch((thing *)pa, dy > 0 ? SO : NO,
						ALIEN, pa->fx, pa->fy, 0,
						pa->cx, pa->cy, 0);
			} else if (dx == dy) {
				if (!psycho(pa))
					launch((thing *)pa, dx > 0 ? SE : NW,
						ALIEN, pa->fx, pa->fy, 0,
						pa->cx, pa->cy, 0);
			} else if (dx == -dy) {
				if (!psycho(pa))
					launch((thing *)pa, dx > 0 ? NE : SW,
						ALIEN, pa->fx, pa->fy, 0,
						pa->cx, pa->cy, 0);
			} else if (dy == 0) {
				if (dx == 0) {
					phaser(pa, pl);
					return;
				}
				if (!psycho(pa))
					launch((thing *)pa, dx > 0 ? EA : WE,
						ALIEN, pa->fx, pa->fy, 0,
						pa->cx, pa->cy, 0);
			}
			if (dx > 0) {
				if (dy > 0)
					pa->cx++;
				else
					pa->cy--;
			} else {
				if (dy > 0)
					pa->cy++;
				else
					pa->cx--;
			}
			return;
		}
		if (dx < 0)
			pa->cx--;
		if (dx > 0)
			pa->cx++;

		if (dy < 0)
			pa->cy--;
		if (dy > 0)
			pa->cy++;
	}
	if (pl->offx > 3 || pl->offx < -3)
		pa->cx += pa->cix;
	else
		pa->cx += pl->offx;
	if (pl->offy > 3 || pl->offy < -3)
		pa->cy += pa->ciy;
	else
		pa->cy += pl->offy;
}

/*
 * Check for psychos -- if found, unleash torpedo barrage
 */
int psycho(pa)
register t_alien *pa;
{
	extern	int rand();
	register char x,y;
	char	ox,
		oy;

	if(((rand()>>5)%6) != 5)	/* magic decision */
		return(0);
	x = pa->cx;
	y = pa->cy;
	ox = pa->fx;
	oy = pa->fy;
	launch((thing *)pa, SO, ALIEN, ox, oy, 0, x, y, 8);
	launch((thing *)pa, NO, ALIEN, ox, oy, 0, x, y, 8);
	launch((thing *)pa, SE, ALIEN, ox, oy, 0, x, y, 8);
	launch((thing *)pa, NW, ALIEN, ox, oy, 0, x, y, 8);
	launch((thing *)pa, NE, ALIEN, ox, oy, 0, x, y, 8);
	launch((thing *)pa, SW, ALIEN, ox, oy, 0, x, y, 8);
	launch((thing *)pa, EA, ALIEN, ox, oy, 0, x, y, 8);
	launch((thing *)pa, WE, ALIEN, ox, oy, 0, x, y, 8);
	return(1);
}

/*
 * Phaser a player for a shanker
 */
void phaser(pa, p)
register t_alien *pa;
register t_player *p;
{
	extern	int rand();
	extern	void pmesg();

	pa->whotoget = 0;
	pa->cix++;
	pa->ciy = (rand()%4)-2;
	p->maxe -= 50;
	if (p->maxe < p->energy)
		p->energy = p->maxe;
	pmesg(p, "Enjoy the shankers revenge!");
}
