#include "rv.h"
#include <ctype.h>

boolean opened_line;	 /* Set TRUE if openline() called */
INT  autoindent;	 /* Physical autoindent, used by insert() */

void
openline(direction)
/*
 *  Openline - Open new line.
 *
 *  If direction < 0, open above current line, else
 *                    open below current line.
 */
INT direction;
{
	register struct li_line	  *line;
	register struct sc_screen *sc;
	register struct wi_window *wi;
	char *s;

	sc = &screen;
	wi = &window;

	file.fi_modified = TRUE;
	xmit_curline();
	opened_line = TRUE;

	/*
	 * Calculate autoindent
	 */
	autoindent = 0;
	if (set_autoindent) {
		s = sc->sc_curline->li_text;
		while (isspace(*s) && *s != '\0')
			if (*s++ == '\t')
				autoindent += set_tabstops;
			else
				autoindent++;
	}

	/*
	 * Send blank line to ed
	 */
	if (direction >= 0) {
		xmit_ed("%da\n\n.\n", sc->sc_lineno);
		sc->sc_lineno++;
		sc->sc_curline++;
	} else
		xmit_ed("%di\n\n.\n", sc->sc_lineno);

	if (sc->sc_curline <= wi->wi_botline ||
			wi->wi_botline < &line_array[NUM_WINDOW_LINES-1]) {
		/*
		 * Case 1,2,3:  Opening line at top of window, within window,
		 *              or bottom of expandable window
		 */

		/*
		 * Scroll off bottom line of window if necessary
		 */
		if (wi->wi_botline >= &line_array[NUM_WINDOW_LINES-1]) {
			if (wi->wi_botline->li_text)
				free(wi->wi_botline->li_text);
			wi->wi_botline->li_text = NULL;
			wi->wi_botline--;
		}

		/*
		 * Slide down window beyond line
		 */
		for (line = wi->wi_botline; line >= sc->sc_curline; --line) {
			(line+1)->li_width = line->li_width;
			(line+1)->li_segments = line->li_segments;
			(line+1)->li_text = line->li_text;
		}
		wi->wi_botline++;
	} else {
		/*
		 * Case 4: Opening line beyond edge of unexpandable window
		 */

		sc->sc_curline--;
		/*
		 * Scroll off topline of window if necessary
		 */
		if (wi->wi_topline <= &line_array[0]) {
			if (wi->wi_topline->li_text)
				free(wi->wi_topline->li_text);
			wi->wi_topline->li_text = NULL;
			wi->wi_topline++;
		}

		/*
		 * Slide up window
		 */
		for (line = wi->wi_topline; line < sc->sc_curline; ++line) {
			(line-1)->li_width = line->li_width;
			(line-1)->li_segments = line->li_segments;
			(line-1)->li_text = line->li_text;
		}
		wi->wi_topline--;
		sc->sc_topline--;
		sc->sc_botline--;
	}

	/*
	 * Create the empty line
	 */
	line = sc->sc_curline;
	line->li_width = 0;
	line->li_segments = 1;
	line->li_text = xalloc(1);
	line->li_text[0] = '\0';

	if (line > wi->wi_botline)
		wi->wi_botline = line;

	file.fi_numlines++;

	/*
	 * Set up for insert()
	 */
	sc->sc_firstcol = 0;
	sc->sc_lastcol = -1;

	/*
	 * Remember opened line for later undo
	 */
	undo.un_validcol = FALSE;
	undo.un_firstline = sc->sc_lineno;
	undo.un_lastline = sc->sc_lineno;
	undo.un_inserted = TRUE;

	/*
	 * Update line to curses
	 */
	if (line > sc->sc_botline) {
		redraw_screen(sc->sc_botline+1);
		move_cursor(sc->sc_lineno, 0);
	} else {
		move_cursor(sc->sc_lineno, 0);
		rv_finsertln(1);
		redraw_screen(sc->sc_botline);
	}
}
