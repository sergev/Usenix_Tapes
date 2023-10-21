#include "rv.h"

void
rv_undo()
/*
 * undo last change
 *
 * undo is accomplished by deleting the last inserted text, and
 * restoring the last deleted text, both of which are assumed
 * to start at the same row,col position.
 */
{
	struct ya_yank	 *yank, *save_yank;
	boolean deleted;
	void rv_linecmd();
	INT direction = -1;

	if (ed_undo) {
		rv_linecmd("u");
		ed_undo = TRUE;
		return;
	}
		
	yank = &yank_array[0];
	save_yank = &yank_array[NUM_YANK_BUFS-1];

	/*
	 * See if there is something to undo
	 */
	if (undo.un_deleted == FALSE && undo.un_inserted == FALSE) {
		flash();
		errflag = 1;
		return;
	}

	/*
	 * Save previous yanked (deleted) text
	 */
	copy((char *)save_yank, (char *)yank, sizeof(struct ya_yank));
	if (yank->ya_text) {
		save_yank->ya_text = xalloc(strlen(yank->ya_text)+1);
		strcpy(save_yank->ya_text, yank->ya_text);
	}
	deleted = undo.un_deleted;
	undo.un_deleted = FALSE;

	if (undo.un_firstline > file.fi_numlines) {
		direction = 1;
		undo.un_firstline = file.fi_numlines;
	}

	move_abs_cursor(undo.un_firstline, 0);
	/*
	 * Delete last inserted text
	 */
	if (undo.un_inserted) {
		register struct sc_screen *sc;

		sc = &screen;
		sc->sc_firstline = undo.un_firstline;
		sc->sc_lastline = undo.un_lastline;
		if (undo.un_validcol == FALSE)
			sc->sc_validcol = FALSE;
		else {
			sc->sc_validcol = TRUE;
			sc->sc_firstcol = undo.un_firstcol;
			sc->sc_lastcol = undo.un_lastcol;
		}
		undo.un_inserted = FALSE;
		delete();
	}
	/*
	 * Restore last deleted text
	 */
	if (deleted && save_yank->ya_type != YANK_EMPTY) {
		move_abs_cursor(undo.un_firstline, 0);
		if (save_yank->ya_type == YANK_COLS)
			screen.sc_column = undo.un_firstcol-1;
		yank_cmd = '$';
		put(direction);
	}

	/*
	 * Free saved yank text
	 */
	if (save_yank->ya_text) {
		free(save_yank->ya_text);
		save_yank->ya_text = NULL;
	}
	save_yank->ya_type = YANK_EMPTY;
}



void
save_Undo()
/*
 * Save current line into Undo buffer
 */
{
	register struct li_line   *line;
	register struct sc_screen *sc;

	sc = &screen;
	line = sc->sc_curline;
	file.fi_modified = TRUE;
	if (sc->sc_origline.li_text == NULL) {
		/*
		 * Copy line into Undo buffer
		 */
		sc->sc_origline.li_text = xalloc(line->li_width+1);
		sc->sc_origline.li_segments = line->li_segments;
		sc->sc_origline.li_width = line->li_width;
		strcpy(sc->sc_origline.li_text, line->li_text);
	}
}


void
toss_undo()
/*
 * Forget last undo operation
 */
{
	struct ya_yank *yank;

	yank = &yank_array[0];
	undo.un_inserted = FALSE;
	undo.un_deleted = FALSE; 
	undo.un_validcol = FALSE;
	if (yank->ya_text) {
		free(yank->ya_text);
		yank->ya_text = NULL;
	}
	yank->ya_type = YANK_EMPTY;
	ed_undo = FALSE;
}
