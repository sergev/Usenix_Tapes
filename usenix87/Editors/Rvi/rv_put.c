#include "rv.h"

extern char *realloc();

void
put(direction)
/*
 * Put buffer at cursor.
 */
INT direction; /* Negative if above cursor */
{
	register struct li_line	  *line;
	register struct ya_yank   *yank;
	register struct sc_screen *sc;
	INT	indx;

	sc = &screen;
	line = sc->sc_curline;

	indx = char_to_yank(yank_cmd);
	if (errflag) {
		flash();
		return;
	}
	yank = &yank_array[indx];

	file.fi_modified = TRUE;
	if (yank->ya_type == YANK_EMPTY) {
		errflag = 1;
		if (yank_cmd == ' ')
			flash();
		else 
			botprint(TRUE, "Register %c is empty", yank_cmd);
		return;
	}
	undo.un_inserted = TRUE;
	undo.un_firstline = undo.un_lastline = sc->sc_lineno;
	undo.un_validcol = FALSE;
	if (yank->ya_type == YANK_COLS) {
		/*
		 * Put text within line
		 */
		register char *s, *s2, *s3;

		save_Undo();
		undo.un_validcol = sc->sc_validcol = TRUE;
		undo.un_firstcol = sc->sc_firstcol = sc->sc_column+1;
		undo.un_lastcol = sc->sc_lastcol = sc->sc_column+yank->ya_width;
		line->li_text = realloc(line->li_text, line->li_width +
			yank->ya_width + 1);
		s = &line->li_text[sc->sc_firstcol];
		s2 = &line->li_text[line->li_width];
		s3 = &line->li_text[line->li_width + yank->ya_width];
		while (s2 >= s)
			*s3-- = *s2--;
		s2 = yank->ya_text;
		while (*s2)
			*s++ = *s2++;
		redraw_curline(line->li_text);
		move_cursor(sc->sc_lineno, sc->sc_firstcol + yank->ya_width-1);
		if (set_fortran && line->li_width > 72) {
			botprint(FALSE, "Line is longer than 72 columns");
			hitcr_continue();
		}
	}
	else { /* Put text between lines */
		sc->sc_validcol = FALSE;
		if (yank->ya_type == YANK_SINGLE) {
			/*
			 * Simple case - put 1 line
			 */
			openline(direction);
			save_Undo();
			redraw_curline(yank->ya_text);
			move_cursor(sc->sc_lineno, COL_FIRST_NONWHITE);
		}
		else {
			/*
			 * put multiple lines
			 */
			xmit_curline();
			xmit_ed("%dr /tmp/yk%d.%d\n", sc->sc_lineno -
				(direction >= 0 ? 0 : 1), getpid(), indx);
			undo.un_lastline = sc->sc_lineno + yank->ya_numlines-1;
			fetch_window(sc->sc_lineno - NUM_WINDOW_LINES/4 -
				LINES/2 + 1, TRUE);
			botprint(FALSE, "%d more lines", yank->ya_numlines);
			hitcr_continue();
		}
	}
}
