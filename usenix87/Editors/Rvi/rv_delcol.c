#include "rv.h"

void delete_columns(first, last)
/*
 *  Delete - delete columns from current line
 */
INT	first,last;
{
	register struct sc_screen *sc;
	register struct li_line   *line;
	register char	*s1, *s2;

	sc = &screen;
	save_Undo();
	line = sc->sc_curline;
	/*
	 * Compact line
	 */
	s1 = &line->li_text[first];
	s2 = &line->li_text[last+1];
	while (*s1++ = *s2++)
		;
	/*
	 * Draw line
	 */
	redraw_curline(line->li_text);
	/*
	 * Adjust cursor
	 */
	sc->sc_column = first;
	if (sc->sc_column >= line->li_width)
		sc->sc_column = line->li_width-1;
	move_cursor(sc->sc_lineno, sc->sc_column);
}
