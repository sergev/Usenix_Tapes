#include "rv.h"

INT	yank_shift = 9;  /* Shift count for numbered yank buffers */
INT	yank_cmd;	 /* "x letter */

INT
char_to_yank(c)
/*
 * Convert a character to a yank ordinal
 */
register INT c;
{
	if (c == ' ')
		return 0;
	else if (c >= '1' && c <= '9')
		c = (yank_shift - (c - '0')) % 9 + 1;
	else if (c >= 'a' && c <= 'z')
		c = c - 'a' + 10;
	else if (c == '.')
		return NUM_YANK_BUFS-2;
	else {
		if (c != '$')
			errflag = 1;
		return NUM_YANK_BUFS-1;
	}
	return c;
}


void
yank()
/*
 * Yank text between first/last marks. 
 * If more than 1 line, text is written to a temp file using ed.
 */
{
	register struct li_line	  *line;
	register struct ya_yank   *yank;
	register struct sc_screen *sc;
	INT	indx;

	sc = &screen;

	indx = char_to_yank(yank_cmd);
	if (errflag) {
		flash();
		return;
	}
	yank = &yank_array[indx];
	line = sc->sc_curline;

	/*
	 * Three cases:  lines, columns, or both
	 */
	if (sc->sc_validcol) { /* If columns */
		if (sc->sc_firstline != sc->sc_lastline) { /* If both */
			botprint(TRUE,
			    "Cant yank columns within multiple lines yet.\n");
			return;
		}
		yank->ya_type = YANK_COLS;
		yank->ya_width = sc->sc_lastcol - sc->sc_firstcol + 1;
		yank->ya_numlines = 0;
		if (yank->ya_text)
			free(yank->ya_text);
		yank->ya_text = xalloc(yank->ya_width+1);
		strncpy(yank->ya_text, &line->li_text[sc->sc_firstcol], yank->ya_width);
		yank->ya_text[yank->ya_width] = '\0';
	}
	else { /* If lines */
		if (sc->sc_firstline == sc->sc_lastline) {
			/*
			 * Simple case - yank line
			 */
			yank->ya_type = YANK_SINGLE;
			yank->ya_width = 0;
			yank->ya_numlines = 1;
			if (yank->ya_text)
				free(yank->ya_text);
			yank->ya_text = xalloc(strlen(line->li_text)+1);
			strcpy(yank->ya_text, line->li_text);
		}
		else {
			/*
			 * Yank multiple lines
			 */
			xmit_curline();
			yank->ya_type = YANK_LINES;
			yank->ya_width = 0;
			yank->ya_numlines = sc->sc_lastline-sc->sc_firstline+1;
			if (yank->ya_text) {
				free(yank->ya_text);
				yank->ya_text = NULL;
			}
			xmit_ed("%d,%dw /tmp/yk%d.%d\n", sc->sc_firstline,
				sc->sc_lastline, getpid(), indx);
			yank->ya_madefile = TRUE;
			if (indx == 0)
				xmit_ed("%d,%dw /tmp/yk%d.%d\n",
					sc->sc_firstline, sc->sc_lastline,
						getpid(), NUM_YANK_BUFS-1);

			botprint(FALSE, "%d lines yanked", yank->ya_numlines);
			hitcr_continue();
		}
	}
}
