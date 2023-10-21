#ifndef lint
static char rcsid[] = "$Header: death.c,v 1.1 84/09/03 00:47:58 lai Exp $";
#endif

#include <curses.h>
#include <math.h>
#include "../h/defs.h"
#include "../h/struct.h"
#include "../h/extern.h"

#ifdef sun
# include "../h/sunscreen.h"
#endif


/*
 *  The player died, draw something on the screen
 *  to show the death.
 */

draw_my_death()
{
#ifdef sun
	if (onsun)
		sun_death();
	else
#endif sun
		ascii_death();
	
}


static
ascii_death()
{
	extern WINDOW	*w_birdv;

#define RADIUS		8
	ascii_explosion(w_birdv, me->u_y, me->u_x * 2, RADIUS, '*');

	draw_maze();
	touchwin(w_birdv);
	wrefresh(w_birdv);
}


static
ascii_explosion(w, r, c, maxr, ch)
	WINDOW		*w;
	register	r, c, maxr, ch;
{
	register	i;

	for (i = 0; i < maxr; i++) {
		mvwaddch(w, r + i, c, ch);
		mvwaddch(w, r + i, c - i * 2, ch);
		mvwaddch(w, r, c - i * 2, ch);
		mvwaddch(w, r - i, c - i * 2, ch);
		mvwaddch(w, r - i, c, ch);
		mvwaddch(w, r - i, c + i * 2, ch);
		mvwaddch(w, r, c + i * 2, ch);
		mvwaddch(w, r + i, c + i * 2, ch);
		wrefresh(w);
		(void) cos((double)0);
	}

#define SPACE	' '
	for (i = 0; i < maxr; i++) {
		mvwaddch(w, r + i, c, SPACE);
		mvwaddch(w, r + i, c - i * 2, SPACE);
		mvwaddch(w, r, c - i * 2, SPACE);
		mvwaddch(w, r - i, c - i * 2, SPACE);
		mvwaddch(w, r - i, c, SPACE);
		mvwaddch(w, r - i, c + i * 2, SPACE);
		mvwaddch(w, r, c + i * 2, SPACE);
		mvwaddch(w, r + i, c + i * 2, SPACE);
		wrefresh(w);
		(void) cos((double)0);
	}
}


#ifdef sun

extern struct pixrect	*px_viewwin;

/*
 *  Draw an explosion with the sun graphics.
 */

static
sun_death()
{
	register int	density, x, y;

#define DENSITY	15

	for (density = DENSITY; density > (DENSITY / 2); density -= 2) {
		for (x = 0; x < VIEWW; x += density)
			rect(px_viewwin, x, 0, 2, VIEWH, OP_SET);
		copyat(px_viewwin, VIEWX, VIEWY, OP_WRITE);
	}
	prclear(px_viewwin);
	copyat(px_viewwin, VIEWX, VIEWY, OP_WRITE);

	sleep(1);

	/* let the newposition redraw the view */
}
#endif sun
