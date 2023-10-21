#ifndef lint
static char rcsid[] = "$Header: player.c,v 1.1 84/08/25 17:11:25 jcoker Exp $";
#endif

#include <stdio.h>
#include "../h/defs.h"
#include "../h/sunscreen.h"

extern struct pixrect	*px_viewwin;

/*
 *  Draw a player the given size at the given position.
 */

draw_other_player(x, y, size, dir)
	register int size;
{
	register	top, bottom, left, right, middle;

	size /= 2;
	top = y - size;
	bottom = y + size;

	size /= 2;
	left = x - size;
	right = x + size;

	middle = bottom - size;

	switch (dir) {
	case DIR_FRONT:
		/* draw eye */
		size /= 5;
		line(px_viewwin, x - size * 2, y, x, y - size, OP_CLEAR);
		line(px_viewwin, x, y - size, x + size * 2, y, OP_CLEAR);
		line(px_viewwin, x + size * 2, y, x, y + size, OP_CLEAR);
		line(px_viewwin, x, y + size, x - size * 2, y, OP_CLEAR);
		size /= 4;
		if (size > 0)
			rect(px_viewwin, x - size / 2, y - size / 2, 
			    size, size, OP_CLEAR);
		/* fall through */

	case DIR_BACK:
		/* player outlines */
		line(px_viewwin, left, bottom, x, top, OP_CLEAR);
		line(px_viewwin, right, bottom, x, top, OP_CLEAR);
		line(px_viewwin, left, bottom, x, middle, OP_CLEAR);
		line(px_viewwin, right, bottom, x, middle, OP_CLEAR);
		break;

	case DIR_RIGHT:
	case DIR_LEFT:
		/* side view */
		size /= 3;
		line(px_viewwin, x, top, x - size, bottom, OP_CLEAR);
		line(px_viewwin, x, top, x + size, bottom, OP_CLEAR);
		line(px_viewwin, x - size, bottom, x + size, bottom, OP_CLEAR);

		/* draw the eye on the side he's facing */
		size /= 2;
		if (dir == DIR_RIGHT) {
			line(px_viewwin, x + size, y - size, x, y, OP_CLEAR);
			line(px_viewwin, x, y, x + size, y + size, OP_CLEAR);
			size /= 3;
			if (size > 0)
				rect(px_viewwin, x + size * 3, y - size / 2,
				    size, size, OP_CLEAR);
		} else {
			line(px_viewwin, x - size, y - size, x, y, OP_CLEAR);
			line(px_viewwin, x, y, x - size, y + size, OP_CLEAR);
			size /= 3;
			if (size > 0)
				rect(px_viewwin, x - size * 3, y - size / 2,
				    size, size, OP_CLEAR);
		}
		break;
	}
}
