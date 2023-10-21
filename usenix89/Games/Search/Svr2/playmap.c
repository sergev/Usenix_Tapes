#ifndef lint
static char rcsid[] = "$Header: playmap.c,v 2.2 85/04/10 22:53:20 matt Exp $";
#endif
/*
 *
 * search
 *
 * multi-player and multi-system search and destroy.
 *
 * Original by Sam Leffler	1981
 * Socket code by Dave Pare	1983
 * Ported & improved
 *      by Matt Crawford	1985
 *
 * routines to manipulate the player map
 *
 * Copyright (c) 1981
 *
 */

#include "defines.h"
#include "structs.h"

/*
 * Screen locations for the players map fields -- if you make
 *  NPLAYER > 16, you'd better change this.
 */
static struct statcoords {
	char	s_x;
	char	s_y;
} scoord[] = {
	{0, 20}, {20, 20}, {40, 20}, {60, 20},
	{0, 21}, {20, 21}, {40, 21}, {60, 21},
	{0, 22}, {20, 22}, {40, 22}, {60, 22},
	{0, 23}, {20, 23}, {40, 23}, {60, 23},
};

/*
 * Update the status of a player on everyone else's screen.
 */
void pldisplay(pl, status)
register t_player *pl;
char *status;
{
	extern	void cput();
	extern	t_player player[NPLAYER];
	register t_player *p;
	register struct statcoords *pcoord = &scoord[pl-player];

	for (p = player; p < &player[NPLAYER]; p++) {
		if (p->status.alive == FALSE || p == pl)
			continue;
		if (pl->status.begin == TRUE) {
			cput(pcoord->s_x, pcoord->s_y, p, "%c[%s]%s ",
				(pl-player)+'A', status, pl->plname);
		} else
			cput(pcoord->s_x+2, pcoord->s_y, p, status);
	}
	pl->status.begin = FALSE;
}

/*
 * Set up the player map for an entering player
 */
void initpdisplay(pl)
register t_player *pl;
{
	extern	void cput();
	extern	t_player player[NPLAYER];
	char	what;
	register t_player *p;
	register struct statcoords *pcoord;

	for (p = player; p < &player[NPLAYER]; p++) {
		if (*p->plname == NULL || p->status.alive == FALSE)
			continue;
		pcoord = &scoord[p-player];
		if (p->status.alive == FALSE) {
			if (p->status.killed == TRUE)
				what = 'd';
			else
				what = 'q';
		}
		else
			what = 'a';
		cput(pcoord->s_x, pcoord->s_y, pl, "%c[%c]: %s",
			(p-player)+'A', what, p->plname);
	}
}
