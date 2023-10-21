#include "rv.h"

void
change()
/*
 *  Change - change text
 */
{
	register struct li_line	  *line;
	register struct sc_screen *sc;
	register struct wi_window *wi;

	sc = &screen;
	wi = &window;

	file.fi_modified = TRUE;
	/*
	 * Three cases:  lines, columns, or both
	 */
	if (sc->sc_validcol) { /* If columns */
		if (sc->sc_firstline != sc->sc_lastline) { /* If both */
			botprint(TRUE,
			    "Cant change columns within multiple lines yet.\n");
			return;
		}
		sc->sc_column = sc->sc_firstcol;
		insert();
	}
	else { /* If lines */
		if (sc->sc_firstline == sc->sc_lastline) {
			/*
			 * Simple case - change 1 line
			 */
			sc->sc_column = 0;
			sc->sc_firstcol = 0;
			sc->sc_lastcol = sc->sc_curline->li_width-1;
			yank_cmd = ' ';
			if (sc->sc_lastcol >= 0) {
				undo.un_deleted = TRUE;
				yank();  /* Save for later undo */
			}
			sc->sc_curline->li_width = 0;
			sc->sc_curline->li_segments = 1;
			sc->sc_curline->li_text[0] = '\0';
			sc->sc_lastcol = -1;
			insert();
		}
		else {
			/*
			 * Change multiple lines
			 */
			delete();
			sc->sc_column = 0;
			if (sc->sc_lineno == file.fi_numlines) /* If bottom */
				if (sc->sc_lineno != 1) /* If not top */
					openline(1);
				else { /* Single line in file, replace */
					sc->sc_firstcol = 0;
					sc->sc_lastcol = -1;
				}
			else
				openline(-1);
			botprint(FALSE, "%d lines changed",
				sc->sc_lastline - sc->sc_firstline + 1);
			insert();
		}
	}
}
