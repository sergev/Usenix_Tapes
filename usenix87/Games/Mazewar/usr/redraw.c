#ifndef lint
static char rcsid[] = "$Header: redraw.c,v 1.1 84/08/25 17:11:27 jcoker Exp $";
#endif

#include <stdio.h>
#include "../h/sunscreen.h"

extern struct pixrect	*px_viewwin,
			*px_statwin,
			*px_mapwin;

/*
 *  Redraw the screen and it's windows.
 */

screen_redraw()
{
	prclear(px_screen);

	linebox(VIEWX - 2, VIEWY - 2, VIEWW + 3, VIEWH + 3);
	copyat(px_viewwin, VIEWX, VIEWY, OP_WRITE);

	linebox(STATX - 2, STATY - 2, STATW + 3, STATH + 3);
	copyat(px_statwin, STATX, STATY, OP_WRITE);

	linebox(MAPX - 2, MAPY - 2, MAPW + 3, MAPH + 3);
	copyat(px_mapwin, MAPX, MAPY, OP_WRITE);

	return(0);
}


/*
 *  Draw an outline box to the given point
 *  and with the given dimensions.
 */

static
linebox(x, y, w, h)
{
	line(px_screen, x, y, x + w, y, OP_CLEAR);
	line(px_screen, x, y, x, y + h, OP_CLEAR);
	line(px_screen, x, y + h, x + w, y + h, OP_CLEAR);
	line(px_screen, x + w, y + h, x + w, y, OP_CLEAR);
}
