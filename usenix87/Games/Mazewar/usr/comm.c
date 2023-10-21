#ifndef lint
static char rcsid[] = "$Header: comm.c,v 1.1 84/08/25 17:11:11 chris Exp $";
#endif

#include <stdio.h>
#include "../h/defs.h"
#include "../h/struct.h"
#include "../h/extern.h"

/*
** If the player can move forward, then move him.
*/
forward()
{
	switch (me->u_dir) {
	case 0:
		if (maze[me->u_x][me->u_y-1] == 0)
		    me->u_y--;
		else
		    return;
		break;
	case 1:
		if (maze[me->u_x+1][me->u_y] == 0)
		    me->u_x++;
		else
		    return;
		break;
	case 2:
		if (maze[me->u_x][me->u_y+1] == 0)
		    me->u_y++;
		else
		    return;
		break;
	case 3:
		if (maze[me->u_x-1][me->u_y] == 0)
		    me->u_x--;
		else
		    return;
		break;
	}
	/*
	** Since we have now successfully moved, let everyone else know
	** about it.
	*/
	sendstat(MOVE);
}

/*  Turn completely backwards */
bturn()
{
	me->u_dir = (me->u_dir + 2) % 4;
#ifdef sun
	/* Only the sun needs to know you have turned so it can redraw you */
	sendstat(0);
#endif
}

/* Turn to the right */
rturn()
{
	me->u_dir = (me->u_dir + 1) % 4;
#ifdef sun
	sendstat(0);
#endif
}

/* Turn to the left */
lturn()
{
	me->u_dir = (me->u_dir + 3) % 4;
#ifdef sun
	sendstat(0);
#endif
}

fire()
{
	register struct user *up;
	struct user *nearest();
#ifdef RANDHIT
	register int dist;
#endif
#ifdef DEBUG
	char s[BUFSIZ];
#endif

	if ((up = nearest(me->u_dir, me->u_x, me->u_y)) == 0)
	    return;
#ifdef RANDHIT
	/* If you are using a random chance to hit, compute it */
	if ((dist = up->u_x - me->u_x + up->u_y - me->u_y) < 0)
	    dist = -dist;
	if ((random() % 100) <  ((30 - dist) * 3))
#endif
	    /* Shoot him */
	    sendact(A_SHOT, up->u_slot, me->u_slot);
#ifdef DEBUG
	if (debug) {
	    (void) sprintf(s, "Shooting %d at %d,%d", up->u_slot, up->u_x, up->u_y);
	    debugs(s);
	}
#endif
}

/* Move one step backwards if possible */
back()
{
	switch (me->u_dir) {
	case 2:
		if (maze[me->u_x][me->u_y-1] == 0)
		    me->u_y--;
		else
		    return;
		break;
	case 3:
		if (maze[me->u_x+1][me->u_y] == 0)
		    me->u_x++;
		else
		    return;
		break;
	case 0:
		if (maze[me->u_x][me->u_y+1] == 0)
		    me->u_y++;
		else
		    return;
		break;
	case 1:
		if (maze[me->u_x-1][me->u_y] == 0)
		    me->u_x--;
		else
		    return;
		break;
	}
	sendstat(MOVE);
}
