#ifndef lint
static char rcsid[] = "$Header: planet.c,v 1.3 85/07/08 17:22:52 matt Exp $";
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
 * routines to manipulate orbiting of the planet
 *
 * Copyright (c) 1979
 *
 * $Log:	planet.c,v $
 * Revision 1.3  85/07/08  17:22:52  matt
 * prepare for preliminary distribution
 * 
 * Revision 1.2  85/02/10  19:19:24  matt
 * Added more realistic orbiting.
 * 
 */

#include "defines.h"
#include "structs.h"

/* orbit increments, indexed by the 0070 (CW) */
/* or 0007 (CCW) bits of an oentry[] element  */

static char omods[8][2] = {
	{ 1, 0}, { 1, 1}, { 0, 1}, {-1, 1},	/* E, SE, S, SW */
	{-1, 0}, {-1,-1}, { 0,-1}, { 1,-1}	/* W, NW, N, NE */
};

/* p->ocnt will be set odd for CCW orbits and always incremented by 2 */

#define ORB_INCR(N)	omods[( (p->ocnt&1) ? (N) : (N)>>3 )&0007]

#define CAN_ENTER_HERE	0200
#define CAN_ORBIT_HERE  0100

#define ORB_DATA(DX,DY)	(oentry[22-(DX)-9*(DY)])

/* the strange subscript formula above makes */
/* the following array correspond to the map */

static char oentry[45]	= {	/* orbital entry calculations */
	0000, 0202, 0303, 0304, 0304, 0304, 0314, 0224, 0000,
	0202, 0373, 0000, 0000, 0000, 0000, 0000, 0315, 0224,
	0371, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0335,
	0260, 0351, 0000, 0000, 0000, 0000, 0000, 0337, 0246,
	0000, 0260, 0350, 0340, 0340, 0340, 0347, 0246, 0000
};

void orbmove(p)
register t_player *p;
{
	extern	void pmesg();
	extern	t_planet planet;
	register char dx, dy, c;

	if((p->ocnt&06) == 4) {
		dx = planet.places[8][0] - p->curx;
		dy = planet.places[8][1] - p->cury;
		if (dx < -4 || dx > 4 || dy < -2 || dy > 2
				|| !((c = ORB_DATA(dx,dy))&CAN_ORBIT_HERE)) {
			/* should "never" happen */
			pmesg(p, "Orbit unstable!\007");
			p->status.orb = FALSE;
			return;
		}
		p->offx = ORB_INCR(c)[0];
		p->offy = ORB_INCR(c)[1];
	} else {
		p->offx = 0;
		p->offy = 0;
	}
	p->ocnt += 2;
}

void orbit(p)
register t_player *p;
{
	extern	void pmesg();
	extern	int  rand();
	extern	t_planet planet;
	register char dx, dy, c;
	int	sense;	/* CW or CCW ? */

	move(STDATAX, STDATAY, p);
	if (p->offx < -1 || p->offx > 1 || p->offy < -1 || p->offy > 1) {
		pmesg(p, "Velocity too great!!");
		return;
	}
	dx = planet.places[8][0] - p->curx;
	dy = planet.places[8][1] - p->cury;
	if (dx < -4 || dx > 4 || dy < -2 || dy > 2
			|| !((c = ORB_DATA(dx,dy))&CAN_ENTER_HERE)) {
		pmesg(p, "Too far from planet to orbit!");
		return;
	}
	if ( ! p->status.orb ) {
		p->status.orb = TRUE;
		p->ocnt = 0;
		sense = dx * p->offy - dy * p->offx;	/* cross product */
		if ( sense > 0 || (sense == 0 && rand()&01) )
			p->ocnt = 1;
		p->offx = p->offy = 0;
		if ( !(c&CAN_ORBIT_HERE) ) {
			/* jump to proper orbit location */
			p->curx += ORB_INCR(c)[0];
			p->cury += ORB_INCR(c)[1];
		}
	}
	pmesg(p, "In orbit around: %s", planet.planetname);
}
