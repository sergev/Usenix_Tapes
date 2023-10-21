#include "rv.h"

void
redraw_screen(firstline)
/*
 * Regenerate the screen from firstline downwards to curses
 *
 * If firstline is NULL, the entire screen is redrawn.
 */
struct	li_line	*firstline;
{
	register struct   li_line	*line;
	register struct	 sc_screen	*sc;
	register struct	 wi_window	*wi;
	INT	 saveline, savecol, seg;

	sc = &screen;
	wi = &window;
	if (firstline == (struct li_line *)0)
		firstline = sc->sc_topline;
	/*
	 * Save cursor location
	 */
	saveline = CURLINE;
	savecol = CURCOLUMN;
	/*
	 * Skip void line segments above topline
	 */
	if (firstline == sc->sc_topline) {
		sc->sc_abovetop = 0; /* Reset topline to real screen top */
		seg = 0;
	} else
		seg = sc->sc_abovetop;
	/*
	 * Skip lines above firstline
	 */
	for (line=sc->sc_topline; line < firstline; ++line)
		seg += line->li_segments;
	/*
	 * Clear screen from firstline downward
	 */
	move(seg,0);
	clrtobot();
	move(seg,0);  /* Because clrtobot homes the cursor in 4bsd */

	/*
	 * Redraw remaining lines
	 */
	if (line <= wi->wi_botline)
		seg += line->li_segments;
	if (seg >= LINES && sc->sc_curline == line) {
		/*
		 * Scroll
		 */
		sc->sc_botline = line-1;
		rv_scroll(1);
		redraw_screen(line+1);
		move(saveline, savecol);
		return;
	}
		
	while (seg < LINES) {
		if (line <= wi->wi_botline) {
			print_line(line->li_text);
			if (CURCOLUMN != 0 || line->li_text[0] == '\0')
				move(CURLINE+1, 0);
			++line;
		}
		if (line > wi->wi_botline) { /* If past bottom of window */
			if ((line - sc->sc_curline) + sc->sc_lineno >
					file.fi_numlines) /* If past eof */
				break;
			/*
			 * Fetch window & redraw everything
			 */
			 if (set_debug > 1)
				 fprintf(stderr, "Forced to redraw\n");
		         fetch_window(sc->sc_lineno - NUM_WINDOW_LINES/4 -
				 LINES/2+1, TRUE);
		         return;
		}
		seg += line->li_segments;
	}
	sc->sc_botline = line-1;

	/*
	 * Append '~' or '@' lines to screen
	 */
	if ((seg = CURLINE) < LINES-1) 
		if (file.fi_numlines > sc->sc_lineno +
				(sc->sc_botline - sc->sc_curline))
			/*
			 * Undisplayed lines
			 */
			while (seg < LINES-1)
				mvaddch(seg++, 0, '@');
		else
			/*
			 * End of file
			 */
			while (seg < LINES-1)
				mvaddch(seg++, 0, '~');
	/*
	 * Restore cursor
	 */
	move(saveline, savecol);
}
