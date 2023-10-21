#include "rv.h"

void
redraw_curline(txt)
/*
 * Redraw the current line using txt.
 * Update rest of screen as necessary
 */
char *txt;
{
	register struct li_line	  *line;
	register struct li_line	*line2;
	register struct sc_screen *sc;
	register INT	seg, i;
	char	*oldtxt;

	sc = &screen;
	line = sc->sc_curline;
	seg = line->li_segments;
	oldtxt = line->li_text;

	/*
	 * Rebuild line internally
	 */
	line->li_width = strlen(txt);
	line->li_segments = 1 + (screen_column(txt, line->li_width-1) +
		set_list) / COLS;
	if (line->li_text != txt) {
		line->li_text = xalloc(line->li_width+1);
		strcpy(line->li_text, txt);
		if (oldtxt)
			free(oldtxt);
	}

	i = sc->sc_abovetop;
	for (line = sc->sc_topline; line < sc->sc_curline; ++line)
		i += line->li_segments;
	move(i, 0);

	if (CURLINE+line->li_segments >= LINES) {
		/*
		 * Line overflowed screen
		 */
		redraw_screen(line);
		return;
	}

	/*
	 * If grew, push down segs below this line
	 */
	if (line->li_segments > seg) {
		i = line->li_segments - seg;
		rv_finsertln(i);
		/* Redraw pushed off segs */
		i = line->li_segments - seg;
		line2 = sc->sc_botline;
		while (i >= 0) {
			i -= line2->li_segments;
			--line2;
		}
		if (line2 <= line)
			line2 = line + 1;
		redraw_screen(line2);
	}

	/*
	 * Clear segments and redraw
	 */
	clrtoeol();
	for (i=line->li_segments; i > 1; --i) {
		move(CURLINE+1, 0);
		clrtoeol();
	}
	/*
	 * Move to first seg of line
	 */
	if (line->li_segments > 1)
		move(CURLINE-line->li_segments+1, 0);

	/*
	 * Draw line
	 */
	i = CURLINE;
	print_line(line->li_text);
	if (seg <= line->li_segments)
		return;

	/*
	 * Shrunk, delete surplus segs
	 */
	move(i+line->li_segments, 0);
	i = seg - line->li_segments;
	rv_fdeleteln(i);
	redraw_screen(sc->sc_botline+1);
}
