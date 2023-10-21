#include "rv.h"
#include <ctype.h>

void
join()
/*
 *  Join current line with line following
 */
{
	register struct li_line	  *line, *line2;
	register struct sc_screen *sc;
	register char *s, *s2;
	struct wi_window *wi;
	struct ya_yank	 *yk;
	INT joincol, bottom;
	char *txt;

	sc = &screen;
	wi = &window;

	file.fi_modified = TRUE;
	line = sc->sc_curline;
	line2 = line+1;
	if (sc->sc_lineno >= file.fi_numlines) {
		flash();
		errflag = 1;
		return;
	}

	if (line2 > wi->wi_botline) {
		fetch_window(sc->sc_lineno - NUM_WINDOW_LINES/4 - LINES/2 + 1,
			TRUE);
		join();
		return;
	}

	txt = xalloc(strlen(line->li_text) + strlen(line2->li_text) + 3);
	strcpy(txt, line->li_text);
	joincol = line->li_width;
	/*
	 * Add a space between lines, if not already there
	 * Add 2 spaces after a period
	 */
	s = txt + line->li_width;
	if (line->li_width > 0 && !isspace(*(s-1))) {
		if (*(s-1) == '.')
			*s++ = ' ';
		*s++ = ' ';
	}

	/*
	 * Strip leading spaces from second line
	 */
	s2 = line2->li_text;
	while (isspace(*s2) && *s2 != '\0')
		++s2;
	
	/*
	 * Append second line to first
	 */
	while (*s2 != '\0')
		*s++ = *s2++;
	*s = '\0';
	openline(-1);
	redraw_curline(txt);
	free(txt);

	/*
	 * Delete old lines
	 * Yank manually to prevent a fetch_window
	 */
	move_abs_cursor(sc->sc_lineno+1, 0);
	sc->sc_validcol = FALSE;
	sc->sc_firstline = sc->sc_lineno;
	sc->sc_lastline = sc->sc_firstline+1;
	yank_cmd = ' ';
	yank();
	yk = &yank_array[0];
	sc->sc_lastline = sc->sc_firstline;
	bottom = (sc->sc_lineno >= file.fi_numlines-1);
	delete();
	delete();
	yk->ya_type = YANK_LINES;
	yk->ya_width = 0;
	yk->ya_numlines = 2;
	if (yk->ya_text) {
		free(yk->ya_text);
		yk->ya_text = NULL;
	}

	/*
	 * Put cursor on spot between joined lines
	 */
	if (!bottom)
		move_cursor(sc->sc_lineno-1, joincol);
	else
		move_cursor(sc->sc_lineno, joincol);
	undo.un_firstline = sc->sc_lineno; /* Bashed by delete() */
	save_Undo();
}
