#ifndef lint
static char rcsid[] = "$Header: scab.c,v 2.1 85/04/10 17:31:37 matt Stab $";
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
 * routines that handle scabs - players deserving of 
 * "special" attention.
 *
 * Copyright (c) 1979
 *
 */

#include "defines.h"
#include "structs.h"


void makescab(p)
register t_player *p;
{
	extern	t_alien alien[NALIEN];
	extern	t_player *whoscab;
	void	enterscab();
	register t_alien *pa;

	p->scabcount++;
	if (p->scabcount > 2)
		enterscab(p);
	for (pa = alien; pa < &alien[NALIEN]; pa++) {
		if (pa->type != SHANK)
			continue;
		pa->aname = NAMESH;
		pa->whotoget = (thing *)p;
	}
	whoscab = p;
}

void seescab(p)
register t_player *p;
{
	extern	t_player *whoscab;
	extern	void pstatus();

	if (whoscab == NOBODY)
		pstatus(p, "Currently no SCAB.");
	else
		pstatus(p, "Current SCAB-- %s", p->plname);
}

static void enterscab(p)
register t_player *p;
{}
