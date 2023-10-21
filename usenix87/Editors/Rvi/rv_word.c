#include "rv.h"
#include <ctype.h>

extern boolean change_flag;	/* The current command is a change cmd */

static INT matchtype;

static boolean
isword(c, bigflag)
/*
 * Return TRUE if c is part of a word
 */
INT c;
boolean bigflag;
{
	INT oldmatchtype;

	if (bigflag)
		return (!isspace(c));
	oldmatchtype = matchtype;
	if (isalpha(c) || isdigit(c) || c == '_') {
		matchtype = 1;
		return (oldmatchtype != 2);
	}
	if (!isspace(c)) {
		matchtype = 2;
		return (oldmatchtype != 1);
	}
	matchtype = 0;
	return FALSE;
}
	


boolean
word_search(c, cmd_count, cmdflag)
/*
 * Scan forward and backward for words
 */
INT c;
INT cmd_count;
boolean cmdflag;	/* TRUE if this is a cursor movement command */
{
	register INT i, len, method;
	register char *s;
	register struct sc_screen *sc;
	INT	direction = 1;
	boolean big_word = FALSE, end_word = FALSE, newline_flag = FALSE;

	sc = &screen;

	switch (c) {
case 'w':
	/*
	 * Scan forward for a word
	 */
	direction = 1;
	big_word = FALSE;
	break;

case 'W':
	/*
	 * Scan forward for a big word
	 */
	direction = 1;
	big_word = TRUE;
	break;

case 'b':
	/*
	 * Scan backward for a word
	 */
	direction = -1;
	big_word = FALSE;
	break;

case 'B':
	/*
	 * Scan backward for a big word
	 */
	direction = -1;
	big_word = TRUE;
	break;

case 'e':
	/*
	 * Scan forward for the end of a word
	 */
	direction = 1;
	big_word = FALSE;
	end_word = TRUE;
	break;

case 'E':
	/*
	 * Scan forward for the end of a big word
	 */
	direction = 1;
	big_word = TRUE;
	end_word = TRUE;
	break;

default:
	return FALSE;

	} /* End switch */


	s = sc->sc_curline->li_text;
	len = sc->sc_curline->li_width;
	sc->sc_validcol = TRUE;
	i = sc->sc_column;
	for (; cmd_count > 0; --cmd_count) {
		/*
		 * Build a search algorithm
		 */
		matchtype = 0;
		if (len <= 0)
			goto beyondline;
		if (newline_flag) {
			method = isword(s[i], big_word) ? 1 : 0;
			newline_flag = FALSE;
		} else {
			INT temptype;

			method = isword(s[i], big_word) ? 2 : 0;
			temptype = matchtype;
			if (direction > 0 && i < len-1 ||
					direction < 0 && i > 0)
				method |= isword(s[i+direction], big_word) ?
					1 : 0;
			matchtype = temptype;
		}
		if (direction < 0 || end_word)
			method |= 4;
		if (method < 4 && change_flag)
			method = 4;  /* Emulating a vi kludge.. */
		else
			switch (method) {
	case 0:
	case 1:			method = 2;
				break;
	case 2:
	case 3:			method = 3;
				break;
	case 7:			method = 4;
				break;
	case 4:
	case 5:			method = 6;
				break;
	case 6:			method = 7;
				break;
			}
		/*
		 * Execute the search algorithm 
		 */
		if (method & 1)
			while (isword(s[i], big_word)) {
				i += direction;
				if (i < 0 || i >= len)
					goto beyondline;
			}
		if (method & 2)
			while (!isword(s[i], big_word)) {
				i += direction;
				if (i < 0 || i >= len)
					goto beyondline;
			}
		if (method & 4)
			for (;;) {
				i += direction;
				if (i < 0 || i >= len ||
						!isword(s[i], big_word)) {
					i -= direction;
					break;
				}
			}
		continue;
beyondline:
		if (cmdflag) {
			move_abs_cursor(sc->sc_lineno+direction, 0);
			s = sc->sc_curline->li_text;
			len = sc->sc_curline->li_width;
			sc->sc_firstline = sc->sc_lineno;
			sc->sc_lastline = sc->sc_lineno;
			if (errflag)
				break;
			if (len == 0 || direction > 0)
				i = 0;
			else
				i = len-1;
			sc->sc_column = sc->sc_firstcol = sc->sc_lastcol = i;
			sc->sc_validcol = TRUE;
			++cmd_count;
			newline_flag = TRUE;
		} else
			break;
	}
	if (direction >= 0) {
		sc->sc_lastcol = i;
		if (!cmdflag && method < 4 && !change_flag)
			sc->sc_lastcol--;
	} else {
		if (!cmdflag)
			sc->sc_lastcol--;
		sc->sc_firstcol = i;
	}
	return (sc->sc_firstcol <= sc->sc_lastcol);
}
