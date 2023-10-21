#include "rv.h"

static INT direction = 1;
static char search_char = ' ';
static boolean incl_char = TRUE;

boolean
forward_back(c, cmd_count)
/*
 * Scan forward and backward in line for character
 */
INT c;
INT cmd_count;
{
	register struct sc_screen *sc;
	register char *s;
	register INT i;

	sc = &screen;
	s = sc->sc_curline->li_text;

	switch (c) {
case 'f':
	/*
	 * Search forward to char
	 */
	search_char = getch();
	direction = 1;
	incl_char = TRUE;
	break;

case 'F':
	/*
	 * Search backward to char
	 */
	search_char = getch();
	direction = -1;
	incl_char = TRUE;
	break;

case 't':
	/*
	 * Search forward upto char
	 */
	search_char = getch();
	direction = 1;
	incl_char = FALSE;
	break;

case 'T':
	/*
	 * Search backward upto char
	 */
	search_char = getch();
	direction = -1;
	incl_char = FALSE;
	break;

case ';':
	/*
	 * Repeat last
	 */
	break;

case ',':
	/*
	 * Repeat last, opposite direction
	 */
	direction = (direction >= 0 ? -1 : 1);
	break;

default:
	return FALSE;

	} /* End switch */


	i = sc->sc_column;
	sc->sc_validcol = TRUE;
	for (; cmd_count > 0; --cmd_count) {
		i += direction;
		while (s[i] != search_char) {
			if (i < 0 || s[i] == '\0')
				return FALSE;
			i += direction;
		}
	}

	if (direction >= 0) {
		sc->sc_lastcol = i;
		if (!incl_char)
			sc->sc_lastcol--;
	} else {
		sc->sc_lastcol--;
		sc->sc_firstcol = i;
		if (!incl_char)
			sc->sc_firstcol++;
	}

	return (sc->sc_firstcol <= sc->sc_lastcol);
}
