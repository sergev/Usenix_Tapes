#include "rv.h"

void
rv_dot()
/*
 * repeat last change
 */
{
	register struct sc_screen *sc;
	register struct ya_yank	  *yk;
	INT direction = -1;
	INT saveline, savecol;

	sc = &screen;
	yk = &yank_array[0];

	/*
	 * See if there is something to repeat
	 */
	if (undo.un_deleted == FALSE && undo.un_inserted == FALSE) {
		flash();
		errflag = 1;
		return;
	}

	/*
	 * Put last inserted text
	 */
	if (undo.un_inserted) {
		saveline = sc->sc_lineno;
		savecol = sc->sc_column;
		/*
		 * Yank text from old location
		 */
		move_abs_cursor(undo.un_firstline, 0);
		sc->sc_firstline = undo.un_firstline;
		sc->sc_lastline = undo.un_lastline;
		if (undo.un_validcol == FALSE)
			sc->sc_validcol = FALSE;
		else {
			sc->sc_validcol = TRUE;
			sc->sc_firstcol = undo.un_firstcol;
			sc->sc_lastcol = undo.un_lastcol;
		}
		yank_cmd = '.';
		yank();
		move_abs_cursor(saveline, savecol);
	}

	/*
	 * Repeat last deletion
	 */
	if (undo.un_deleted && yk->ya_type != YANK_EMPTY) {
		sc->sc_firstline = sc->sc_lineno;
		if (yk->ya_type != YANK_COLS) {
			sc->sc_validcol = FALSE;
			sc->sc_lastline = sc->sc_firstline + yk->ya_numlines-1;
		}
		else {
			sc->sc_validcol = TRUE;
			sc->sc_lastline = sc->sc_firstline;
			sc->sc_firstcol = sc->sc_column;
			sc->sc_lastcol = sc->sc_firstcol + yk->ya_width - 1;
			if (sc->sc_lastcol >= sc->sc_curline->li_width)
				sc->sc_lastcol = sc->sc_curline->li_width-1;
			if (sc->sc_lastcol < 0) {
				flash();
				errflag = 1;
				return;
			}
		}
		yank_cmd = ' ';
		delete();
	}

	/*
	 * Repeat last insertion
	 */
	if (undo.un_inserted) {
		yank_cmd = '.';
		if (undo.un_validcol == TRUE)
			/*
			 * Dot inserts changes before the cursor
			 */
			sc->sc_column--;
		put(direction);
	}
}
