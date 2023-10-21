#include "rv.h"

void
rv_scroll_backward(n)
/*
 * Scroll n lines backward
 * Must be within window
 * Cursor location is undefined
 */
register n;
{
	register struct li_line   *line, *newline;
	register struct sc_screen *sc;
	register INT	seg;
	register struct wi_window *wi;
	register struct fi_file	  *fi;

	sc = &screen;
	wi = &window;
	fi = &file;
	seg = 0;
	for (line = sc->sc_topline-1; n > 0; --n, --line) {
		if (line < wi->wi_topline) {
			errflag = 1;
			botprint(TRUE,
			    "rv_scroll_backward - past top of window\n\n");
			hitcr_continue();
			return;
		}
		seg += line->li_segments;
	}
	++line;
	move(0,0);
	seg -= sc->sc_abovetop;
	rv_finsertln(seg);
	move(0,0);
	for (newline = line; newline < sc->sc_topline; ++newline) {
		print_line(newline->li_text);
		if (CURCOLUMN != 0 || newline->li_text[0] == '\0')
			move(CURLINE+1, 0);
	}
	xmit_curline();
	sc->sc_lineno = sc->sc_lineno - (sc->sc_curline - line);
	sc->sc_topline = line;
	sc->sc_curline = line;
	sc->sc_column = 0;
	sc->sc_abovetop = 0;

	/*
	 * Compute bottom line
	 */
	for (seg = line->li_segments; seg < LINES;
			seg += line->li_segments) {
		++line;
		if (line > wi->wi_botline)
			break;
	}
	sc->sc_botline = line-1;

	if (line <= wi->wi_botline)
		seg -= line->li_segments;
	move(seg, 0);
	clrtobot();
	move(seg, 0); /* Because clrtobot homes the cursor in 4bsd */
	/*
	 * Append '~' or '@' lines to screen
	 */
	if (seg < LINES-1) 
		if (fi->fi_numlines > sc->sc_lineno +
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
	move(0,0);
}
