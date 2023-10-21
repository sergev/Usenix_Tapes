#include "rv.h"

void
rv_scroll(n)
/*
 * Scroll n lines
 * Must be within window
 * Cursor is set to last line
 */
register INT n;
{
	register struct li_line   *line, *oldline;
	register struct sc_screen *sc;
	register INT	seg,newseg,i;
	register struct wi_window *wi;

	sc = &screen;
	wi = &window;
	/*
	 * Set cursor to bottom line+1
	 */
	newseg = 0;
	for (line = sc->sc_topline; line <= sc->sc_botline; ++line)
		newseg += line->li_segments;
	newseg += sc->sc_abovetop;
	move(newseg,0);
	clrtobot();
	/*
	 * Calculate number of segments to scroll in
	 */
	seg = 0;
	for (i=n; i > 0 && line <= wi->wi_botline; --i, ++line)
		seg += line->li_segments;
	seg -= LINES-newseg-1;
	move(LINES-1,0);
	rv_fscroll(seg);
	move(newseg-seg,0);
	/*
	 * Scroll in new line(s)
	 */
	for (line = sc->sc_botline+1; n > 0; --n, ++line) {
		if (line > wi->wi_botline) {
			errflag = 1;
			botprint(TRUE, "rv_scroll - past bottom of window\n\n");
			hitcr_continue();
			return;
		}
		print_line(line->li_text);
		if (CURCOLUMN != 0 || line->li_text[0] == '\0') {
			if (CURLINE == LINES-1) {
				move(LINES-1, 0);
			} else
				move(CURLINE+1, 0);
		}
	}
	/*
	 * Adjust screen params
	 */
	xmit_curline();
	--line;
	sc->sc_lineno += (line - sc->sc_curline);
	sc->sc_botline = line;
	sc->sc_curline = line;
	sc->sc_column = 0;

	/*
	 * Compute new top line
	 */
	for (seg = line->li_segments; seg < LINES;
			seg += line->li_segments) {
		--line;
		if (line < wi->wi_topline)
			break;
	}
	sc->sc_topline = line+1;
	/*
	 * Compute # of displayed segs above top line
	 */
	if (line >= wi->wi_topline)
		sc->sc_abovetop = LINES-1 - (seg - line->li_segments);
}
