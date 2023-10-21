#ifndef lint
static char rcsid[] = "$Header: groups.c,v 1.3 85/08/06 22:29:38 matt Exp $";
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
 * routines to execute player "team" commands l, q, j, g
 *
 * Copyright (c) 1979
 *
 * $Log:	groups.c,v $
 * Revision 1.3  85/08/06  22:29:38  matt
 * Change handling of "r", "b", "g", "j", "q" commands to
 * provide better feedback, using per-player message buffer.
 * 
 */

#include "defines.h"
#include "structs.h"

/*
 * Look at your group's members.
 */
void lookg(p)
register t_player *p;
{
	extern	void cput(),
		clearline(),
		put();
	extern	t_player player[NPLAYER];
	extern	char visual[NPLAYER][NPLAYER];
	register t_player *pl;
	register int i;

	cput(MSDATA, p, "From security-- group = ");
	clearline(p);
	for (pl = player; pl < &player[NPLAYER]; pl++) {
		if (pl->status.alive == FALSE)
			continue;
		if (visual[p-player][i = pl-player])
			put(p, "%c ", i+'A');
	}
}

/*
 * Join a group.
 */
void joing(pl, person)
register t_player *pl;
char person;
{
	extern	t_player player[NPLAYER];
	extern	char visual[NPLAYER][NPLAYER];
	register int i;
	register t_player *p;

	if (person == '\n' || person == '\r') {
		pl->cmdpend = 0;
		prompt(pl, "");
		return;
	}
	if (person >= 'a')
		person -= 'a' - 'A';
	for (p = player; p < &player[NPLAYER]; p++) {
		if (p->status.alive == FALSE || p == pl)
			continue;
		if ((i = p-player) == person-'A') {
			visual[pl-player][i] = 1;
			return;
		}
	}
}

/*
 * Quit a group.
 */
void quitg(pl, person)
register t_player *pl;
char person;
{
	extern	t_player player[NPLAYER];
	extern	char visual[NPLAYER][NPLAYER];
	register int i,
		j;
	register t_player *p;

	if (person == '\n' || person == '\r') {
		pl->cmdpend = 0;
		prompt(pl, "");
		return;
	}
	if (person == '.') {
		j = pl-player;
		for (i = 0; i < NPLAYER; i++)
			visual[j][i] = 0;
		pl->cmdpend = 0;
		prompt(pl, "");
		return;
	}
	if (person >= 'a')
		person -= 'a' - 'A';
	for (p = player; p < &player[NPLAYER]; p++) {
		if (p->status.alive == FALSE || p == pl)
			continue;
		if ((i = p-player) == person-'A') {
			visual[pl-player][i] = 0;
			return;
		}
	}
}

/*
 * Send a group message.
 */
void groupm(pl, c)
register t_player *pl;
char c;
{
	extern	void makescab(),
		pmesg();
	extern	t_player player[NPLAYER];
	extern	char visual[NPLAYER][NPLAYER];
	register t_player *p;
	register int j;
	char	cc;

	if (c == '\n' || c == '\r') {
		j = pl-player;
		cc = j+'A';
		for (p = player; p < &player[NPLAYER]; p++)
			if (visual[j][p-player] && p->status.alive == TRUE) {
				pmesg(p, "From %c-- %s", cc, pl->mesgbuf);
			}
		pl->cmdpend = 0;
		donemsg(pl);
		return;
	}
	if (c == SCABLETTER) {		/* Benedict Arnold */
		c = '^';
		makescab(pl);
	}
	echomsg(pl, c);
}
