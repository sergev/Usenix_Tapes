#include "rv.h"


void
fetch_window(first, redraw_flag)
/*
 * Fetch a window starting at line 'first' in the file.
 * Adjust current line if necessary
 * Redraw screen
 */
INT first;
boolean redraw_flag;	/* TRUE if redraw requested */
{
	register struct li_line   *line;
	register struct sc_screen *sc;
	register struct wi_window *wi;
	char buf[512];
	INT	last, i, j;
	extern alarmring();

	sc = &screen;
	wi = &window;

	xmit_curline();
	/*
	 * Free old window's text
	 */
	if (file.fi_numlines > 0)
		for (line=wi->wi_topline; line <= wi->wi_botline; ++line)
			if (line->li_text) {
				free(line->li_text);
				line->li_text = NULL;
			}
	
	/*
	 * Set first/last lines
	 */
	if (first < 1)
		first = 1;
	last = first + NUM_WINDOW_LINES-1;
	if (last > file.fi_numlines)
		last = file.fi_numlines;

retry:
	/*
	 * Send request to ed
	 */
	xmit_sync();
	xmit_ed("$=\n");
	xmit_ed("%d,%dp\n", first, last);

	(void) recv_sync(FALSE);
	alarm(RV_TIMEOUT);
	(void) fgets(buf, 511, file.fi_fpin);

	if ((i = atoi(buf)) == 0 && buf[0] != '0')
		panic("$= request to ed failed\n\n");

	if (i <= 0) {
		/*
		 * Deleted whole file
		 */
		wi->wi_topline = &line_array[NUM_WINDOW_LINES/4];
		wi->wi_botline = wi->wi_topline;
		wi->wi_topline->li_text = xalloc(1);
		wi->wi_topline->li_text[0] = '\0';
		wi->wi_topline->li_segments = 1;
		wi->wi_topline->li_width = 0;
		sc->sc_curline = wi->wi_topline;
		sc->sc_lineno = 1;
		sc->sc_column = 0;
		sc->sc_abovetop = 0;
		sc->sc_topline = sc->sc_botline = sc->sc_curline;
		file.fi_numlines = 1;
		alarm(0);
		redraw_screen((struct li_line *)0);
		xmit_ed("0a\n\n.\n");
		return;
	}

	file.fi_numlines = i;
	if (last > i) {
		/*
		 * The file shrunk below our requested window size.
		 * Probably due to a .,$d command.
		 *
		 * Shift the window towards the top of whatever
		 * text is left.
		 */
		last = i;
		first = last - (NUM_WINDOW_LINES+1);
		if (first < 1)
			first = 1;
		goto retry;
	}
	/*
	 * See if file grew and window can be extended
	 */
	j = first + NUM_WINDOW_LINES-1;
	if (j > file.fi_numlines)
		j = file.fi_numlines;
	if (j != last) {
		last = j;
		goto retry;
	}

	/*
	 * Read in window lines
	 */
	line = &line_array[0];
	for (i=first; i <= last; ++i, ++line) {
		if (fgets(buf, 511, file.fi_fpin) == NULL)
			panic("Error during input from ed\n\n");
		/*
		 * Strip trailing \n
		 */
		if ((j = strlen(buf)) > 0 && buf[j-1] == '\n')
			buf[--j] = '\0';
		line->li_text = xalloc(j+1);
		strcpy(line->li_text, buf);
		line->li_width = j;
		line->li_segments = 1 + (screen_column(buf, j-1)+set_list)/COLS;
	}
	alarm(0);

	wi->wi_topline = &line_array[0];
	wi->wi_botline = line-1;
	sc->sc_abovetop = 0;
	if (sc->sc_lineno >= first && sc->sc_lineno <= last) {
		/*
		 * Case 1 - Current line is still in window
		 *          Calculate curline, topline
		 */
		line = wi->wi_topline + (sc->sc_lineno - first);
		i = sc->sc_curline - sc->sc_topline;
		if (sc->sc_lineno - i < first)
			sc->sc_topline = wi->wi_topline;
		else
			sc->sc_topline = line - i;
		sc->sc_curline = line;
	} else {
		/*
		 * Case 2 - Current line outside window
		 *          Choose new current line, topline
		 */
		line = wi->wi_topline + NUM_WINDOW_LINES/4 + LINES/2 - 1;
		if (line > wi->wi_botline)
			line = wi->wi_botline;
		sc->sc_topline = line - (LINES/2 - 1);
		if (sc->sc_topline < wi->wi_topline ||
		    sc->sc_topline > wi->wi_botline)
			sc->sc_topline = wi->wi_topline;
		sc->sc_lineno = first + (line - wi->wi_topline);
		sc->sc_curline = line;
	}

	if (redraw_flag) { /* If redraw requested */
		redraw_screen((struct li_line *)0);
		move_cursor(sc->sc_lineno, COL_FIRST_NONWHITE);
	} else {
		/*
		 * Compute bottom line
		 */
		line = sc->sc_topline;
		for (j = line->li_segments; j < LINES; j += line->li_segments) {
			++line;
			if (line > wi->wi_botline)
				break;
		}
		sc->sc_botline = line-1;
	}
}
