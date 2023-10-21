#include "rv.h"
#include <ctype.h>

void
read_where_mod(cmd, specified_count, cmd_count)
/*
 * Read in where modifier, override old count if specified
 */
INT	cmd;              /* Previous cmd_character */
boolean	specified_count;  /* TRUE if count was specified  */
INT	cmd_count;	  /* Command count, default 1 */
{
	INT	c;
	INT	count;
	void	where_mod();

	if (read_cmd(&c, &count)) {
		specified_count = TRUE;
		cmd_count = count;
	}
	/*
	 * Doubled character means single line (internally stored as '\0')
	 */
	if (c == cmd)
		c = '\0';
	where_mod(c, specified_count, cmd_count, FALSE);
}


void
where_mod(c, specified_count, cmd_count, cmdflag)
/*
 * Set the (row,col) pairs to cover the range of text specified by
 * the where modifier.  The caller gets the pairs via the screen structure.
 *
 * Screen structure,
 *
 *	sc->sc_firstline	First line # of range
 *	sc->sc_firstcol		First column # of range
 *
 *	sc->sc_lastline		Last line # of range
 *	sc->sc_lastcol		Last column # of range
 *
 *	sc->sc_validcol		If column # if valid (otherwise
 *				the range covers whole lines)
 */
INT	c;			/* Command character */
boolean	specified_count;	/* TRUE if count was specified */
INT	cmd_count;		/* Command count, default 1*/
boolean	cmdflag;		/* TRUE if this is a cursor movement command */
{
	register struct	sc_screen *sc;
	boolean forward_back(), word_search(), search();
	static	INT	last_searchdir = 1;

	sc = &screen;
	sc->sc_firstline = sc->sc_lineno;
	sc->sc_firstcol = sc->sc_column;
	sc->sc_lastline = sc->sc_lineno;
	sc->sc_lastcol = sc->sc_column;
	sc->sc_validcol = TRUE;

	switch (c) {

case 'h':
case '\b':
#ifdef USG
case KEY_LEFT:
#endif
	/*
	 * Left cursor
	 */
	sc->sc_firstcol -= cmd_count;
	sc->sc_lastcol--;
	break;

case 'l':
case ' ':
#ifdef USG
case KEY_RIGHT:
#endif
	/*
	 * Right cursor
	 */
	sc->sc_lastcol += cmd_count-1;
	break;


case 'k':
case '-':
case CTRL(P):
#ifdef USG
case KEY_UP:
#endif
	/*
	 * Up cursor
	 */
	sc->sc_validcol = FALSE;
	sc->sc_firstline -= cmd_count;
	break;

case 'j':
case '\n':
case '\r':
case '+':
case CTRL(N):
#ifdef USG
case KEY_DOWN:
#endif
	/*
	 * Down cursor
	 */
	sc->sc_validcol = FALSE;
	sc->sc_lastline += cmd_count;
	break;

case '\0':
	/*
	 * Repeated character (line count)
	 */
	sc->sc_validcol = FALSE;
	sc->sc_lastline += cmd_count-1;
	break;

case '0':
	/*
	 * Beginning of line
	 */
	sc->sc_lastcol--;
	sc->sc_firstcol = 0;
	break;

case '^':
case '_':
	/*
	 * First nonwhite char
	 */
	sc->sc_lastcol--;
	sc->sc_firstcol = 0;
	while (isspace(sc->sc_curline->li_text[sc->sc_firstcol]) &&
			sc->sc_firstcol != sc->sc_lastcol)
		sc->sc_firstcol++;
	if (sc->sc_firstcol > sc->sc_lastcol)
		sc->sc_lastcol = sc->sc_firstcol;
	break;

case '$':
	/*
	 * End of line
	 */
	sc->sc_lastcol = sc->sc_curline->li_width-1;
	break;

case ',': 
case ';':
case 'f':
case 'F':
case 't':
case 'T':
	/*
	 * Search for character forward and backward
	 */
	if (!forward_back(c, cmd_count))
		goto error;
	break;
	
case 'w':
case 'W':
case 'b':
case 'B':
case 'e':
case 'E':
	/*
	 * Go to the next word
	 */
	if (!word_search(c, cmd_count, cmdflag))
		goto error;
	break;

case 'H':
#ifdef USG
case KEY_HOME:
#endif
	/*
	 * Home line
	 */
	sc->sc_validcol = FALSE;
	sc->sc_firstline = sc->sc_lineno - (sc->sc_curline-sc->sc_topline) +
		cmd_count-1;
	break;

case 'L':
#ifdef USG
case KEY_LL:
#endif
	/*
	 * Last line
	 */
	sc->sc_validcol = FALSE;
	sc->sc_lastline = sc->sc_lineno + (sc->sc_botline-sc->sc_curline) -
		cmd_count+1;
	break;

case 'G':
	/*
	 * Goto line #
	 */
	sc->sc_validcol = FALSE;
	if (!specified_count)
		cmd_count = file.fi_numlines;
	if (cmd_count < sc->sc_lineno)
		sc->sc_firstline = cmd_count;
	else
		sc->sc_lastline = cmd_count;
	break;

case '|':
	/*
	 * Goto column #
	 */
	cmd_count = file_column(sc->sc_curline->li_text, cmd_count-1);
	if (cmd_count == sc->sc_column)
		goto error;
	if (cmd_count < sc->sc_column)
		sc->sc_firstcol = cmd_count;
	else
		sc->sc_lastcol = cmd_count;
	break;

case '\'':
	/*
	 * Goto mark
	 */
	c = getch();
	xmit_ed("'%c\n", c);
	xmit_sync();
	xmit_ed(".=\n");
	if (recv_sync(FALSE)) {
		char buf[32];

		sc->sc_validcol = FALSE;
		fgets(buf, 31, file.fi_fpin);
		if ((c = atoi(buf)) > 0)
			if (c < sc->sc_lineno)
				sc->sc_firstline = c;
			else
				sc->sc_lastline = c;
	} else
		goto error;
	break;
	
case '/':
	/*
	 * Search forward
	 */
	mvaddch(LINES-1, 0, '/');
	last_searchdir = 1;
	if (!search(1, rv_getline(), cmdflag)) {
		errflag = 1;
		return;
	}
	errflag = cmdflag;
	break;

case '?':
	/*
	 * Search backward
	 */
	mvaddch(LINES-1, 0, '?');
	last_searchdir = -1;
	if (!search(-1, rv_getline(), cmdflag)) {
		errflag = 1;
		return;
	}
	errflag = cmdflag;
	break;

case 'N':
	/*
	 * Reverse last search
	 */
	if (!search(last_searchdir > 0 ? -1 : 1, "", cmdflag)) {
		errflag = 1;
		return;
	}
	errflag = cmdflag;
	break;

case 'n':
	/*
	 * Repeat last search
	 */
	if (!search(last_searchdir, "", cmdflag)) {
		errflag = 1;
		return;
	}
	errflag = cmdflag;
	break;

case '[':
	/*
	 * Search backward for C function
	 */
	if (getch() != '[' || !search(-1, "^{", cmdflag)) {
		move(LINES-1, 0);
		clrtoeol();
		move_cursor(sc->sc_lineno, sc->sc_column);
		goto error;
	}
	errflag = cmdflag;
	break;

case ']':
	/*
	 * Search forward for C function
	 */
	if (getch() != ']' || !search(1, "^{", cmdflag)) {
		move(LINES-1, 0);
		clrtoeol();
		move_cursor(sc->sc_lineno, sc->sc_column);
		goto error;
	}
	errflag = cmdflag;
	break;


default:
	/*
	 * Unknown where modifier
	 */
	goto error;


	}  /* End of switch */

	if (errflag)
		return;
	/*
	 * Consistency checks
	 */
	if (sc->sc_validcol) {
		if (sc->sc_firstcol < 0) {
			if (sc->sc_column == 0)
				goto error;
			sc->sc_firstcol = 0;
		}
		if (sc->sc_lastcol < 0) 
			goto error;
		if (sc->sc_firstline == sc->sc_lineno)
			if (sc->sc_firstcol >= sc->sc_curline->li_width)
				goto error;
		if (sc->sc_lastline == sc->sc_lineno)
			if (sc->sc_lastcol >= sc->sc_curline->li_width) {
				if (sc->sc_column == sc->sc_curline->li_width)
					goto error;
				sc->sc_lastcol = sc->sc_curline->li_width;
			}
	}
	if (sc->sc_firstline > 0 && sc->sc_firstline <= file.fi_numlines &&
	    sc->sc_lastline > 0 &&  sc->sc_lastline <= file.fi_numlines)
		return;

error:
	errflag = 1;
	flash();
	return;
}
