#include "rv.h"

#ifndef USG
extern int _putchar();
static char *rCM, *rSC, *rRC, *rCS, *rAL, *rDL, *rSR;
static char tcbuf[256];
static char buf[4096];
#endif

/*
 * Fast versions of curses routines
 */

rv_finit()
{
#ifndef USG
	char *s;
	extern char *tgetstr();

	if (!ttytype || *ttytype == '\0')
		panic("Please set TERM to your terminal type.\n");
	s = tcbuf;
	tgetent(buf, ttytype);
	rSC = tgetstr("sc", &s);
	rCM = tgetstr("cm", &s);
	rRC = tgetstr("rc", &s);
	rCS = tgetstr("cs", &s);
	rAL = tgetstr("al", &s);
	rDL = tgetstr("dl", &s);
	rSR = tgetstr("sr", &s);

	if (!rCM)
		panic("Your terminal needs cursor motion capability");
#endif
}


static
unchange(win)
/*
 * Reset window so it appears not to have changed
 */
WINDOW *win;
{
	register INT	y, maxy;

	maxy = win->_maxy;
	for (y=0; y < maxy; ++y)
		win->_firstch[y] = _NOCHANGE;
}


rv_fscroll(cnt)
/*
 * Fast scroll.  Cursor is assumed to be in the lower left corner.
 */
INT cnt;
{
#ifdef USG
	for (; cnt > 0; --cnt) {
		if (scroll(stdscr) == ERR)
			return ERR;
	}
#else
	refresh();
	for (; cnt > 0; --cnt) {
		if (scroll(stdscr) == ERR)
			return ERR;
		if (scroll(curscr) == ERR)
			return ERR;
		curscr->_cury++;
	}
	unchange(stdscr);
#endif
	return OK;
}


rv_finsertln(cnt)
/*
 * Fast line insert.
 */
INT cnt;
{
#ifdef USG
	for (; cnt > 0; --cnt) {
		if (winsertln(stdscr) == ERR)
			return ERR;
	}
#else
	INT i;

	refresh();
	if (CURLINE == 0 && rSR)
		for (i=0; i < cnt; ++i)
			tputs(rSR, 1, _putchar);
	else if (rAL) 
		for (i=0; i < cnt; ++i)
			tputs(rAL, cnt - CURLINE, _putchar);
	else if (rCS) {
		tputs(rSC, 1, _putchar); /* Stupid chng scroll homes cursor */
		tputs(tgoto(rCS, LINES-1, CURLINE), 1, _putchar);
		tputs(rRC, 1, _putchar);
		for (i=0; i < cnt; ++i)
			tputs(rSR, 1, _putchar);
		tputs(tgoto(rCS, LINES-1, 0), 1, _putchar);
		tputs(rRC, 1, _putchar);
	} else
		panic("Need insert line capability");

	for (; cnt > 0; --cnt) {
		if (winsertln(stdscr) == ERR)
			return ERR;
		if (winsertln(curscr) == ERR)
			return ERR;
	}
	unchange(stdscr);
#endif
	return OK;
}


rv_fdeleteln(cnt)
/*
 * Fast line delete.
 */
INT cnt;
{
#ifdef USG
	for (; cnt > 0; --cnt) {
		if (wdeleteln(stdscr) == ERR)
			return ERR;
	}
#else
	INT i;

	refresh();
	if (rDL)
		for (i=0; i < cnt; ++i)
			tputs(rDL, cnt-CURLINE, _putchar);
	else if (rCS) {
		tputs(rSC, 1, _putchar); /* Stupid scroll homes cursor */
		tputs(tgoto(rCS, LINES-1, CURLINE), 1, _putchar);
		tputs(tgoto(rCM, 0, LINES-1), 1, _putchar);
		for (i=0; i < cnt; ++i)
			tputs("\n", 1, _putchar);
		tputs(tgoto(rCS, LINES-1, 0), 1, _putchar);
		tputs(rRC, 1, _putchar);
	} else
		panic("Need delete line capability");
	for (; cnt > 0; --cnt) {
		if (wdeleteln(stdscr) == ERR)
			return ERR;
		if (wdeleteln(curscr) == ERR)
			return ERR;
	}
	unchange(stdscr);
#endif

	return OK;
}
