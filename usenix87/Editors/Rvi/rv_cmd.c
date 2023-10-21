#include "rv.h"
#include <ctype.h>
/*
 * rv_cmd.c  -  Main command loop
 */

boolean	change_flag;		/* Used by word_scan() */

rv_cmd()
/*
 * Loop and execute commands
 * The screen is always refreshed afterwards.
 */
{
	register struct sc_screen *sc;
	INT	i,j,k;	/* temp vars */
	INT     c;		/* Command character */
	boolean specified_count;/* TRUE if count was explicitly specified */
	INT     cmd_count;	/* Count, default 1 */
	void	read_where_mod(), where_mod(), rv_undo(), rv_dot(), rv_debug();
	void	rv_mark();
	boolean	search();

	sc = &screen;

for (;;) {   /* Loop forever */
	errflag = 0;
	yank_cmd = ' ';
	specified_count = read_cmd(&c, &cmd_count);

	/*
	 * Main command switch
	 */
	switch (c) {

case CTRL(L):
#ifdef USG
case KEY_CLEAR:
#endif
	/*
	 * Clear and redisplay screen
	 */
	 clearok(curscr, TRUE);
	 break;

case CTRL(R):
	/*
	 * Redraw screen
	 */
	 clearok(curscr, TRUE);
	 redraw_screen((struct li_line *)0);
	 break;

case CTRL(E):
#ifdef USG
case KEY_SF:
#endif
	/*
	 * Scroll up one line.  Leave cursor
	 * on same line, if possible.
	 */
	i = sc->sc_lineno + (CURLINE == 0 ? 1 : 0);
	j = sc->sc_column;
	/* Force scroll 1 line */
	move_abs_cursor((sc->sc_botline - sc->sc_curline) + sc->sc_lineno+1, 0);
	move_abs_cursor(i, j); /* Restore cursor */
	break;

case CTRL(Y):
#ifdef USG
case KEY_SR:
#endif
	/*
	 * Scroll down one line.  Leave cursor
	 * on same line, if possible.
	 */
	i = sc->sc_lineno - (CURLINE == LINES-2 ? 1 : 0);
	j = sc->sc_column;
	/* Scroll reverse scroll 1 line */
	move_abs_cursor(sc->sc_lineno-1 - (sc->sc_curline - sc->sc_topline), 0);
	move_abs_cursor(i, j);
	break;

case 'h':
case '\b':
#ifdef USG
case KEY_LEFT:
case KEY_BACKSPACE:
#endif
	/*
	 * Move cursor left 1 character
	 */
	move_cursor(sc->sc_lineno, sc->sc_column-cmd_count);
	break;

case 'l':
case ' ':
#ifdef USG
case KEY_RIGHT:
#endif
	/*
	 * Move cursor right 1 character
	 */
	move_cursor(sc->sc_lineno, sc->sc_column+cmd_count);
	break;

case 'k':
case CTRL(P):
#ifdef USG
case KEY_UP:
#endif
	/*
	 * Move cursor up 1 character
	 */
	move_abs_cursor(sc->sc_lineno-cmd_count, COL_SAME);
	break;

case '-':
	/*
	 * Move to first nonwhite character on previous line
	 */
	 move_abs_cursor(sc->sc_lineno-cmd_count, COL_FIRST_NONWHITE);
	 break;

case 'j':
case '\n':
case CTRL(N):
#ifdef USG
case KEY_DOWN:
#endif
	/*
	 * Move cursor down 1 character
	 */
	move_abs_cursor(sc->sc_lineno+cmd_count, COL_SAME);
	break;

case '\r':
case '+':
	/*
	 * Move to first nonwhite character on next line
	 */
	move_abs_cursor(sc->sc_lineno+cmd_count, COL_FIRST_NONWHITE);
	break;

case 'x':
#ifdef USG
case KEY_DC:
#endif
	/*
	 * Delete character under cursor
	 */
	toss_undo();
	if (sc->sc_curline->li_width <= 0) {
		errflag = 1;
		flash();
		break;
	}
	sc->sc_validcol = TRUE;
	sc->sc_firstline = sc->sc_lineno;
	sc->sc_firstcol  = sc->sc_column;
	sc->sc_lastline = sc->sc_lineno;
	sc->sc_lastcol = sc->sc_column+cmd_count-1;
	if (sc->sc_lastcol >= sc->sc_curline->li_width)
		sc->sc_lastcol = sc->sc_curline->li_width-1;
	delete();
	break;

case 'X':
	/*
	 * Delete character before cursor
	 */
	toss_undo();
	if (sc->sc_column <= 0) {
		errflag = 1;
		flash();
		break;
	}
	sc->sc_validcol = TRUE;
	sc->sc_lastline = sc->sc_lineno;
	sc->sc_lastcol = sc->sc_column-1;
	sc->sc_firstline = sc->sc_lineno;
	sc->sc_firstcol = sc->sc_column-cmd_count;
	if (sc->sc_firstcol < 0)
		sc->sc_firstcol = 0;
	delete();
	break;

case CTRL(G):
	/*
	 * Print file size
	 */
	sizemsg();
	break;

case CTRL(U):
#ifdef USG
case KEY_PPAGE:
#endif
	/*
	 * Scroll screen up 1 page
	 */
	if (specified_count)
		set_scroll = cmd_count;
	else
		cmd_count = set_scroll;
	if (sc->sc_lineno <= 1) { /* If at top of file */
		errflag = 1;
		flash();
		break;
	}
	k = sc->sc_curline - sc->sc_topline;
	move_cursor(sc->sc_lineno - k, 0); /* Goto top of screen */
	for (i=0; i < cmd_count; ++i) {  /* Scrolling loop */
		if (sc->sc_lineno <= 1)  /* If past top of file */
			break;
		move_abs_cursor(sc->sc_lineno-1, 0); /* Scroll up */
		if (errflag)
			break;
	}
	if (i >= cmd_count) 
		move_abs_cursor(sc->sc_lineno + k, 0);  /* Move cursor back */
	move_cursor(sc->sc_lineno, COL_FIRST_NONWHITE); /* Set column */
	break;

case CTRL(D):
#ifdef USG
case KEY_NPAGE:
#endif
	/*
	 * Scroll screen down a page
	 */
	if (specified_count)
		set_scroll = cmd_count;
	else
		cmd_count = set_scroll;
	if (sc->sc_lineno >= file.fi_numlines) { /* If past bottom of file */
		errflag = 1;
		flash();
		break;
	}
	k = sc->sc_botline - sc->sc_curline;
	move_cursor(sc->sc_lineno + k, 0); /* Goto bottom of screen */
	for (i=0; i < cmd_count; ++i) { /* Scrolling loop */
		if (sc->sc_lineno >= file.fi_numlines) /* If past end of file */
			break;
		move_abs_cursor(sc->sc_lineno+1, 0); /* Scroll */
		if (errflag)
			break;
	}
	if (i >= cmd_count) 
		move_abs_cursor(sc->sc_lineno - k, 0);  /* Move cursor back */
	move_cursor(sc->sc_lineno, COL_FIRST_NONWHITE); /* Set column */
	break;

case CTRL(B):
	/*
	 * Go back a page, with two
	 * lines of continuity
	 */
	if (sc->sc_lineno <= 1) { /* If past top of file */
		flash();
		errflag = 1;
		break;
	}
	clearok(curscr, TRUE);
	/* Go to top of screen */
	move_cursor(sc->sc_lineno - (sc->sc_curline - sc->sc_topline), 0);
	/* Go back LINES-2 */
	i = sc->sc_lineno - cmd_count*(LINES-2) + LINES/2;
	if (i <= 0)
		i = 1;
	move_abs_cursor(i, COL_FIRST_NONWHITE);
	break;

case CTRL(F):
	/*
	 * Go forward a page, with two
	 * lines of continuity
	 */
	if (sc->sc_lineno >= file.fi_numlines) { /* if past end of file */
		flash();
		errflag = 1;
		break;
	}
	clearok(curscr, TRUE);
	/* Go to bottom of screen */
	move_cursor(sc->sc_lineno + (sc->sc_botline - sc->sc_curline), 0);
	/* Go forward LINES-2 */
	i = sc->sc_lineno + cmd_count*(LINES-2) - LINES/2;
	if (i > file.fi_numlines)
		i = file.fi_numlines;
	move_abs_cursor(i, COL_FIRST_NONWHITE);
	break;

case 'M':
	/*
	 * Position cursor to middle line
	 */
	move_abs_cursor(sc->sc_lineno-(sc->sc_curline-sc->sc_topline)+LINES/2,
		COL_FIRST_NONWHITE);
	break;

case 'Q':
	/*
	 * Drop into ed
	 */
	quit();
	break;

case 'y':
	/*
	 * Yank
	 */
	toss_undo();
	read_where_mod(c, specified_count, cmd_count);
	if (errflag)
		break;
	yank();
	break;

case 'Y':
	/*
	 * Yank lines
	 */
	toss_undo();
	sc->sc_validcol = FALSE;
	sc->sc_firstline = sc->sc_lineno;
	sc->sc_lastline = sc->sc_lineno + cmd_count-1;
	if (sc->sc_lastline > file.fi_numlines)
		sc->sc_lastline = file.fi_numlines;
	yank();
	break;

case 'p':
	/*
	 * Put buffer
	 */
	undo.un_deleted = FALSE;
	j = sc->sc_lineno+1;
	put(1);
	if (cmd_count > 1)
		if (sc->sc_validcol)
			dup_insert(cmd_count);
		else {
			for (i=1; i < cmd_count; ++i)
				put(1);
			undo.un_firstline = j;
		}
	break;

case 'P':
	/*
	 * Put buffer above cursor
	 */
	undo.un_deleted = FALSE;
	j = sc->sc_lineno-1;
	put(-1);
	if (cmd_count > 1)
		if (sc->sc_validcol)
			dup_insert(cmd_count);
		else {
			for (i=1; i < cmd_count; ++i)
				put(-1);
			undo.un_lastline = j;
		}
	break;

case 'd':
	/*
	 * Delete
	 */
	toss_undo();
	read_where_mod(c, specified_count, cmd_count);
	if (errflag)
		break;
	delete();
	break;

case 'D':
#ifdef USG
case KEY_EOL:
#endif
	/*
	 * Delete rest of line
	 */
	toss_undo();
	sc->sc_validcol = TRUE;
	sc->sc_firstline = sc->sc_lineno;
	sc->sc_lastline = sc->sc_lineno;
	sc->sc_firstcol = sc->sc_column;
	sc->sc_lastcol = sc->sc_curline->li_width-1;
	delete();
	break;

#ifdef USG
case KEY_DL:
#else
case -2:
#endif
	/*
	 * Delete line
	 */
	toss_undo();
	sc->sc_validcol = FALSE;
	sc->sc_firstline = sc->sc_lineno;
	sc->sc_lastline = sc->sc_lineno;
	delete();
	break;

case 'c':
	/*
	 * Change
	 */
	toss_undo();
	change_flag = TRUE;
	read_where_mod(c, specified_count, cmd_count);
	if (errflag)
		break;
	change();
	change_flag = FALSE;
	break;

case 'C':
	/*
	 * Change rest of line
	 */
	toss_undo();
	sc->sc_firstline = sc->sc_lineno;
	sc->sc_lastline = sc->sc_lineno + cmd_count-1;
	if (sc->sc_lastline == sc->sc_firstline) {
		sc->sc_validcol = TRUE;
		sc->sc_firstline = sc->sc_lineno;
		sc->sc_firstcol = sc->sc_column;
		sc->sc_lastcol = sc->sc_curline->li_width-1;
	} else {
		sc->sc_validcol = FALSE;
		if (sc->sc_lastline > file.fi_numlines)
			sc->sc_lastline = file.fi_numlines;
	}
	change();
	break;
	
case 'i':
#ifdef USG
case KEY_IC:
#endif
	/*
	 * Insert before cursor
	 */
	toss_undo();
	sc->sc_validcol = TRUE;
	sc->sc_firstcol = sc->sc_column;
	sc->sc_lastcol = sc->sc_column-1;
	sc->sc_firstline = sc->sc_lastline = sc->sc_lineno;
	insert();
	if (cmd_count > 1)
		dup_insert(cmd_count);
	break;

case 'I':
	/*
	 * Insert before first nonwhite char
	 */
	toss_undo();
	move_cursor(sc->sc_lineno, COL_FIRST_NONWHITE);
	sc->sc_validcol = TRUE;
	sc->sc_firstcol = sc->sc_column;
	sc->sc_lastcol = sc->sc_column-1;
	sc->sc_firstline = sc->sc_lastline = sc->sc_lineno;
	insert();
	if (cmd_count > 1)
		dup_insert(cmd_count);
	break;

case 'a':
	/*
	 * Append after cursor
	 */
	toss_undo();
	if (sc->sc_curline->li_width > 0)
		sc->sc_column++;
	input_mode = TRUE; /* So move_cursor() will let us go past eol */
	move_cursor(sc->sc_lineno, sc->sc_column);
	sc->sc_validcol = TRUE;
	sc->sc_firstcol = sc->sc_column;
	sc->sc_lastcol = sc->sc_column-1;
	sc->sc_firstline = sc->sc_lastline = sc->sc_lineno;
	insert();
	if (cmd_count > 1)
		dup_insert(cmd_count);
	break;

case 'A':
	/*
	 * Append at end of line
	 */
	toss_undo();
	sc->sc_column = sc->sc_curline->li_width;
	input_mode = TRUE; /* So move_cursor() will let us go past eol */
	move_cursor(sc->sc_lineno, sc->sc_column);
	sc->sc_validcol = TRUE;
	sc->sc_firstcol = sc->sc_column;
	sc->sc_lastcol = sc->sc_column-1;
	sc->sc_firstline = sc->sc_lastline = sc->sc_lineno;
	insert();
	if (cmd_count > 1)
		dup_insert(cmd_count);
	break;

case 'R':
	/*
	 * Replace text
	 */
	toss_undo();
	sc->sc_validcol = TRUE;
	sc->sc_firstcol = sc->sc_column;
	sc->sc_lastcol = sc->sc_column-1;
	sc->sc_firstline = sc->sc_lastline = sc->sc_lineno;
	replace_flag = TRUE;
	insert();
	replace_flag = FALSE;
	if (cmd_count > 1)
		dup_insert(cmd_count);
	break;

case '~':
	/*
	 * Switch case of char
	 */
case 'r':
	/*
	 * Replace character under cursor
	 */
	toss_undo();
	if (sc->sc_curline->li_width <= 0) { /* If empty line */
		errflag = 1;
		flash();
		break;
	}
	if (c != '~')
		while ((i = getch()) == '\0')
			;
	else {
		i = sc->sc_curline->li_text[sc->sc_column];
		if (isupper(i))
			i = tolower(i);
		else if (islower(i))
			i = toupper(i);
	}
	save_Undo();
	sc->sc_validcol = TRUE;
	sc->sc_firstline = sc->sc_lastline = sc->sc_lineno;
	sc->sc_firstcol = sc->sc_lastcol = sc->sc_column;
	undo.un_deleted = TRUE;
	undo.un_inserted = TRUE;
	undo.un_validcol = sc->sc_validcol = TRUE;
	undo.un_firstline = undo.un_lastline = sc->sc_lineno;
	undo.un_firstcol = undo.un_lastcol = sc->sc_column;
	yank();
	sc->sc_curline->li_text[sc->sc_column] = i;
	redraw_curline(sc->sc_curline->li_text);
	move_cursor(sc->sc_lineno, sc->sc_column);
	if (cmd_count > 1)
		dup_insert(cmd_count);
	if (c == '~' && sc->sc_column < sc->sc_curline->li_width-1)
		move_cursor(sc->sc_lineno, sc->sc_column+1);
	break;

case 's':
	/*
	 * Substitute 1 character
	 */
	toss_undo();
	sc->sc_validcol = TRUE;
	sc->sc_firstcol = sc->sc_column;
	sc->sc_lastcol = sc->sc_column+cmd_count-1;
	sc->sc_firstline = sc->sc_lastline = sc->sc_lineno;
	if (sc->sc_lastcol >= sc->sc_curline->li_width)
		sc->sc_lastcol = sc->sc_curline->li_width - 1;
	insert();
	break;

case 'U':
	/*
	 * Undo changes to current line
	 */
	toss_undo();
	if (sc->sc_origline.li_text == NULL) { /* If no changes */
		errflag = 1;
		flash();
		break;
	}
	redraw_curline(sc->sc_origline.li_text);
	move_cursor(sc->sc_lineno, COL_FIRST_NONWHITE);
	sc->sc_origline.li_text = NULL;
	break;

case 'u':
	/*
	 * General undo
	 */
	rv_undo();
	break;

case '.':
	/*
	 * Repeat last change
	 */
	for (i=0; i < cmd_count; ++i)
		rv_dot();
	break;

case 'J':
	/*
	 * Join lines
	 */
	toss_undo();
	join();
	break;


case 'o':
	/*
	 * Open line after cursor
	 */
	toss_undo();
	openline(1);
	insert();
	break;

case 'O':
#ifdef USG
case KEY_IL:
#endif
	/*
	 * Open line before cursor
	 */
	toss_undo();
	openline(-1);
	insert();
	break;

case 'S':
	/*
	 * Substitute lines
	 */
	toss_undo();
	sc->sc_validcol = FALSE;
	sc->sc_firstline = sc->sc_lineno;
	sc->sc_lastline = sc->sc_lineno+cmd_count-1;
	if (sc->sc_lastline > file.fi_numlines) {
		errflag = 1;
		flash();
		break;
	}
	change();
	break;

case 'm':
	/*
	 * Mark text
	 */
	rv_mark();
	break;

case ':':
	/*
	 * Line command
	 */
	mvaddch(LINES-1, 0, ':');
	rv_linecmd(rv_getline());
	break;

case '&':
	/* 
	 * Repeat last s/../../
	 */
	rv_linecmd("s//%/");
	break;
	
case CTRL(V):
	/*
	 * Display debug info
	 */
	rv_debug();
	break;

case 'Z':
	/*
	 * Write and quit
	 */
	if (getch() != 'Z') {
		errflag = 1;
		flash();
		break;
	}
	rv_linecmd("wq");
	break;
		
default:
	/*
	 * A cursor movement command
	 */
	where_mod(c, specified_count, cmd_count, TRUE);
	if (errflag)
		break;
	/*
	 * Determine which row,col pair is the new location (first or last?)
	 */
	j = COL_FIRST_NONWHITE;
	if (sc->sc_firstline == sc->sc_lineno &&
	   (!sc->sc_validcol || sc->sc_firstcol == sc->sc_column)) {
		i = sc->sc_lastline;
		if (sc->sc_validcol)
			j = sc->sc_lastcol;
	} else {
		i = sc->sc_firstline;
		if (sc->sc_validcol)
			j = sc->sc_firstcol;
	}
	if (i == sc->sc_lineno) /* If already on line */
		move_cursor(i,j);
	else
		move_abs_cursor(i,j);
	break;
		

	} /* End of switch */

	opened_line = FALSE;

	fflush(file.fi_fpout);
	hitcr_continue();
	refresh();

}   /* End of loop */

}
