#include "rv.h"
#include <ctype.h>

/*
 * rv_move.c  - Cursor motion routines
 */

void
move_abs_cursor(lineno, column)
/*
 * Move cursor to absolute position in file.  Try to stay within the window
 * if possible.
 */
INT lineno, column;
{
	register INT	seg;
	register struct li_line	*line, *newline;
	register struct sc_screen      *sc;
	register struct wi_window      *wi;
	INT	 oldlineno, offset;
	struct   fi_file	       *fi;
	void     rv_scroll(), rv_scroll_backward();

	sc = &screen;
	wi = &window;
	fi = &file;
	errflag = 0;
	/*
	 * Boundary checks
	 */
	if (lineno < 1) { /* If past top of file */
		errflag = 1;
		flash();
		return;
	} else if (lineno > fi->fi_numlines) { /* If past bottom of file */
		errflag = 1;
		flash();
		return;
	}

	if (lineno < sc->sc_lineno - (sc->sc_curline-sc->sc_topline)) {
		/*
		 * Past top of screen
		 */
		oldlineno = sc->sc_lineno;
		if (lineno < sc->sc_lineno - (sc->sc_curline-wi->wi_topline)) {
			/*
			 * Past top of window, fetch more data
			 */
			fetch_window(lineno-NUM_WINDOW_LINES/4-LINES/2+1,
									FALSE);
			if (errflag)
				return;
		}
		newline = sc->sc_curline - (sc->sc_lineno - lineno);
		seg = 0;
		if (oldlineno == sc->sc_lineno) { /* If relatively close */
			/*
			 * Find distance, in segments, above top of screen
			 */
			for (line = newline; line < sc->sc_topline; ++line)
				seg += line->li_segments;
			if (seg <= LINES/3+1) { /* If very close */
				/*
				 * Scroll backwards
				 */
				rv_scroll_backward(sc->sc_topline - newline);
				move_cursor(lineno, column);
				return;
			}
		}

		/*
		 * Newline is too far away for scrolling,
		 * so we redraw the screen and deposit newline
		 * in the center.
		 */

		/*
		 * Set the top of the screen at LINES/2 segments above newline
		 */
		xmit_curline();
		seg = LINES/2;
		for (line = newline; line >= wi->wi_topline && seg > 0; --line)
			seg -= line->li_segments;
		sc->sc_topline = line+1;
		sc->sc_curline = newline;
		sc->sc_lineno = lineno;
		sc->sc_abovetop = 0;
		/*
		 * Compute bottom line
		 */
		line = sc->sc_topline;
		for (seg = line->li_segments; seg < LINES;
				seg += line->li_segments) {
			++line;
			if (line > wi->wi_botline)
				break;
		}
		sc->sc_botline = line-1;

		/*
		 * Update the screen to curses
		 */
		move_cursor(lineno, column);
		redraw_screen((struct li_line *)0);
		return;
	} /* End of past top of screen */
		

	if (lineno > sc->sc_lineno + (sc->sc_botline - sc->sc_curline)) {
		/*
		 * Past bottom of screen
		 */
		oldlineno = sc->sc_lineno;
		offset = sc->sc_botline - sc->sc_curline;
		if (lineno > sc->sc_lineno + (wi->wi_botline-sc->sc_curline)) {
			/*
			 * Past bottom of window
			 */
			fetch_window(lineno-NUM_WINDOW_LINES/4-LINES/2+1,
									TRUE);
			if (errflag)
				return;
		}
		newline = sc->sc_curline + (lineno - sc->sc_lineno);
		seg = 0;
		if (oldlineno == sc->sc_lineno) { /* If relatively close */
			/*
			 * Find distance in segments past bottom of screen
			 */
			for (line = sc->sc_curline+offset+1; line <= newline;
									++line)
				seg += line->li_segments;
			if (seg <= LINES/3+1 && newline >= sc->sc_botline) {
				/*
				 * Scroll screen forwards
				 */
				rv_scroll(newline - sc->sc_botline);
				move_cursor(lineno, column);
				return;
			}
		}

		/*
		 * Newline is too far away for scrolling,
		 * so we redraw the screen and deposit newline
		 * in the center.
		 */

		/*
		 * Set the top of the screen at LINES/2 segments above newline
		 */
		xmit_curline();
		seg = LINES/2;
		for (line = newline; line >= wi->wi_topline && seg > 0; --line)
			seg -= line->li_segments;
		sc->sc_abovetop = 0;
		sc->sc_topline = line+1;
		sc->sc_curline = newline;
		sc->sc_lineno = lineno;
		/*
		 * Compute bottom line
		 */
		line = sc->sc_topline;
		for (seg = line->li_segments; seg < LINES;
				seg += line->li_segments) {
			++line;
			if (line > wi->wi_botline)
				break;
		}
		sc->sc_botline = line-1;

		/*
		 * Update the screen to curses
		 */
		move_cursor(lineno, column);
		redraw_screen((struct li_line *)0);
		return;
	} /* End of past bottom of screen */
		
	/*
	 * Otherwise, newline must already be on the screen
	 */

	move_cursor(lineno, column);
	return;
}



void
move_cursor(lineno, column)
/*
 * Move cursor to position in file.  Must already be on screen.
 */
INT lineno, column;
{
	register INT	seg;
	register struct sc_screen	*sc;
	register struct	li_line		*line, *newline;
	INT      linewidth;
	static	 INT	prev_column;

	errflag = 0;
	sc = &screen;
	newline = sc->sc_curline + (lineno - sc->sc_lineno);
	/*
	 * Boundary checks
	 */
	if (newline < sc->sc_topline || newline > sc->sc_botline) {
		errflag = 1;
		botprint(TRUE,"move_cursor, lineno %d beyond screen\n", lineno);
		return;
	}

	if (column == COL_SAME)
		column = file_column(newline->li_text, prev_column);
	else {
		prev_column = -1;
		if (column == COL_FIRST_NONWHITE) {
			register char *s;

			s = newline->li_text;
			while (*s != '\0' && isspace(*s))
				++s;
			column = s - newline->li_text;
		}
	}

				
	if (column < 0) { /* If past left side of screen */
		if (sc->sc_column == 0) { /* Already at left side? */
			errflag = 1;
			flash();
			return;
		}
		column = 0;
	}

	if (lineno != sc->sc_lineno && sc->sc_origline.li_text != NULL)
		xmit_curline();

	/*
	 * Compute screen column and segment #
	 */
	sc->sc_curline = newline;
	sc->sc_lineno = lineno;
	/*
	 * Limit column to end of line (or line+1 if input mode)
	 */
	linewidth = newline->li_width + (input_mode ? 1 : 0);
	if (column >= linewidth && column > 0) {
		column = (linewidth == 0 ? 0 : linewidth-1);
		if (sc->sc_column >= column) /* If already at edge */
			flash();
	}

	sc->sc_column = column;
	column = screen_column(newline->li_text, column);
	if (prev_column < 0)
		prev_column = column;
	if (input_mode && sc->sc_column == linewidth-1 && sc->sc_column != 0)
		++column;

	seg = 0;
	for (line=sc->sc_topline; line < newline; ++line)
		seg += line->li_segments;
	seg += column / COLS + sc->sc_abovetop;
	column %= COLS;
	if (seg >= LINES) {
		errflag = 1;
		botprint(TRUE, "move_cursor, seg %d beyond screen\n", seg);
		return;
	}
	/*
	 * Update cursor to curses
	 */
	move(seg, column);
}
