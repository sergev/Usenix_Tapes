#include "rv.h"

/*
 * Miscellaneous subroutines
 */

char *xalloc(n)
/*
 * Allocate and zero n bytes
 */
{
	char *s;
	char *malloc();

	if ((s = malloc(n)) == NULL) {
		write(2, "Out of memory", 13);
		endwin();
		exit(1);
	}
	zero(s, n);
	return s;
}


boolean did_botprint;  /* True if botprint() was already called */
boolean scrolled;      /* True if screen scrolled from a call */

/*VARARGS1*/
void
botprint(standout_flag, txt, arg1, arg2, arg3, arg4, arg5)
/*
 * Printf line(s) on bottom of screen with the given attribute(s).
 */
INT  standout_flag;
char *txt, *arg1, *arg2, *arg3, *arg4, *arg5;
{
	char buf[512];
	INT  oldrow, oldcol, l;

	oldrow = CURLINE;
	oldcol = CURCOLUMN;

	sprintf(buf, txt, arg1, arg2, arg3, arg4, arg5);
	l = strlen(buf);
	if (l > 0 && buf[l-1] == '\n') /* Remove trailing \n */
		buf[--l] = '\0';
	move(LINES-1, 0);
	if (did_botprint) {
		rv_fscroll(1);
		move(LINES-1, 0);
		scrolled = TRUE;
	}
	else
		clrtoeol();

	if (standout_flag)
		standout();
	addstr(buf);
	if (standout_flag)
		standend();
	while (l > 0 && buf[l-1] == '\n') {
		buf[--l] = '\0';
		/* scroll(stdscr); */
		scrolled = TRUE;
	}

	did_botprint = TRUE;

	if (l >= COLS || l > CURCOLUMN)
		scrolled = TRUE;

	move(oldrow, oldcol);
}


hitcr_continue()
{
	INT oldrow, oldcol;
	INT c;

	if (scrolled) {
		/*
		 * Screen scrolled.  Redraw
		 */
		oldrow = CURLINE;
		oldcol = CURCOLUMN;
		move(LINES-1,0);
		rv_fscroll(1);
		move(LINES-1, 0);
		standout();
		addstr("[Hit return to continue]");
		standend();
		refresh();
		while ((c = getch()) != '\r' && c != '\n' && c != ' ')
			;
		redraw_screen((struct li_line *) 0);
		move(oldrow, oldcol);
		scrolled = FALSE;
	}
	did_botprint = FALSE;
}


void
panic(msg)
/*
 * Print msg and abort
 */
char *msg;
{
	botprint(TRUE, "Fatal error: %s\n", msg);
	move(LINES-1, 0);
	rv_fscroll(1);
	move(LINES-1, 0);
	refresh();
	endwin();
	exit(1);
}

void
sizemsg()
{
	if (file.fi_name[0] == '\0' || strcmp(file.fi_name, "/dev/null") == 0)
		botprint(0, "No file  line %d of %d --%d%%--", 
			screen.sc_lineno, file.fi_numlines, 
			(screen.sc_lineno*100) / file.fi_numlines);
	else
		botprint(0, "\"%s\" %sline %d of %d --%d%%--", file.fi_name,
			file.fi_modified ? "[modified] " : "",
			screen.sc_lineno, file.fi_numlines, 
			(screen.sc_lineno*100) / file.fi_numlines);
}


void
rv_debug()
{
	botprint(FALSE, "Absolute:\n");
	botprint(FALSE, "wi_topline=%d, wi_botline=%d\n",
		window.wi_topline - &line_array[0],
		window.wi_botline - &line_array[0]);
	botprint(FALSE, "sc_topline=%d, sc_botline=%d, sc_curline=%d\n",
		screen.sc_topline - &line_array[0],
		screen.sc_botline - &line_array[0],
		screen.sc_curline - &line_array[0]);
	botprint(FALSE, "sc_lineno=%d, fi_numlines=%d\n",
		screen.sc_lineno, file.fi_numlines);
	botprint(FALSE, "Relative to window:\n");
	botprint(FALSE, "sc_topline=%d, sc_botline=%d, sc_curline=%d\n",
		screen.sc_topline - window.wi_topline,
		screen.sc_botline - window.wi_topline,
		screen.sc_curline - window.wi_topline);
}
