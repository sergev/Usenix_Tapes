#include "rv.h"

void delete()
/*
 *  Delete - delete text
 */
{
	register struct li_line	  *line;
	register struct sc_screen *sc;
	register struct wi_window *wi;
	void	delete_columns();
	INT seg;

	sc = &screen;
	wi = &window;

	file.fi_modified = TRUE;

	if (sc->sc_firstline != sc->sc_lastline || !sc->sc_validcol)
		xmit_curline();
	/*
	 * Save text in unnamed buffer for 'p' command.
	 */
	if (yank_cmd != ' ') { /* If user also specified buf */
		yank();	/* Yank into it too */
		yank_cmd = ' ';
	}
	yank(); /* Yank to the unnamed buf */
	undo.un_deleted = TRUE;
	undo.un_validcol = FALSE;
	undo.un_firstline = sc->sc_firstline;

	/*
	 * Three cases:  lines, columns, or both
	 */
	if (sc->sc_validcol) { /* If columns */
		if (sc->sc_firstline != sc->sc_lastline) { /* If both */
			botprint(TRUE,
			    "Cant delete columns within multiple lines yet.\n");
			return;
		}
		undo.un_validcol = TRUE;
		undo.un_firstcol = sc->sc_firstcol;
		delete_columns(sc->sc_firstcol, sc->sc_lastcol);
	}
	else { /* If lines */
		/*
		 * Store deleted lines in numbered (1-9) yank buffers
		 */
		++yank_shift;  /* Shift numbered bufs */
		yank_cmd = '1';
		yank(); /* Yank to new buf 1 */

		if (sc->sc_firstline == sc->sc_lastline) {
			/*
			 * Case 1 - delete single line
			 */
			xmit_ed("%dd\n", sc->sc_firstline);
			move_abs_cursor(sc->sc_lineno, COL_SAME);
			seg = sc->sc_curline->li_segments;
			if (sc->sc_curline->li_text) {
				free(sc->sc_curline->li_text);
				sc->sc_curline->li_text = NULL;
			}
			for (line=sc->sc_curline+1; line <= wi->wi_botline;
			   ++line) {
				/*
				 * Compact window
				 */
				(line-1)->li_text = line->li_text;
				(line-1)->li_width = line->li_width;
				(line-1)->li_segments = line->li_segments;
			}
			wi->wi_botline->li_text = NULL;
			wi->wi_botline--;
			sc->sc_botline--;
			file.fi_numlines--;
			if (sc->sc_curline > wi->wi_botline) {
				sc->sc_curline--;
				sc->sc_lineno--;
			}
			if (sc->sc_curline < sc->sc_topline) {
				sc->sc_abovetop = 0;
				sc->sc_topline = sc->sc_curline;
			} else {
				rv_fdeleteln(seg);
				move_cursor(sc->sc_lineno, COL_SAME);
				redraw_screen(sc->sc_botline+1);
				return;
			}
			move_cursor(sc->sc_lineno, COL_SAME);
			line = sc->sc_curline;
		} else {
			/*
			 * Delete multiple lines
			 */
			xmit_ed("%d,%dd\n",sc->sc_firstline,sc->sc_lastline);
			file.fi_numlines -= sc->sc_lastline-sc->sc_firstline+1;
			if (file.fi_numlines > 0)
				fetch_window(sc->sc_lineno - NUM_WINDOW_LINES/4
					- LINES/2 + 1, FALSE);
			else /* deleted all lines, wipe window clean */
				for (line = wi->wi_topline;
					    line <= wi->wi_botline; ++line) {
					free(line->li_text);
					line->li_text = NULL;
				}
			line = NULL;
		}
		if (file.fi_numlines <= 0) { 
			/*
			 * Deleted whole file.  Set up initial window
			 */
			wi->wi_topline = &line_array[NUM_WINDOW_LINES/4];
			wi->wi_botline = wi->wi_topline;
			wi->wi_topline->li_text = xalloc(1);
			wi->wi_topline->li_text[0] = '\0';
			wi->wi_topline->li_segments = 1;
			wi->wi_topline->li_width = 0;
			sc->sc_curline = wi->wi_topline;
			sc->sc_lineno = 1;
			sc->sc_column = 0;
			sc->sc_abovetop = 0;
			sc->sc_topline = sc->sc_botline = sc->sc_curline;
			file.fi_numlines = 1;
			line = NULL;
			xmit_ed("0a\n\n.\n");
			redraw_screen(line);
			move_cursor(sc->sc_lineno, COL_SAME);
			botprint(FALSE, "Empty file");
			hitcr_continue();
			return;
		}

		redraw_screen(line);
		move_cursor(sc->sc_lineno, COL_SAME);
		if (sc->sc_lastline - sc->sc_firstline > 1) {
			botprint(FALSE, "%d lines deleted",
				sc->sc_lastline - sc->sc_firstline + 1);
			hitcr_continue();
		}
	}
}
