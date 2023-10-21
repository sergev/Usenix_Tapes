/*
**	A look-alike to help all us poor fools who did'nt buy ATT's System
**	V.
*/

#include <curses.h>

wnoutrefresh(win)
WINDOW *win;
{
	wrefresh(win);
}
