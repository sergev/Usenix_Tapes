#include "rv.h"
#include <ctype.h>

#define ALLOC_LEN  64  /* Size of reallocation chunk */

boolean replace_flag;		/* TRUE if R command */
boolean superquote;		/* Set by rv_getchar if char is quoted by ^V */
extern INT autoindent;		/* Physical autoindent, if opened_line */

       char *fake_input;	/* Set this to fake text into rv_getchar */
static char fake_buf[512];	/* Buffer for faked characters */
extern char *realloc();


void
insert()
/*
 * Insert input at the current cursor position, replacing
 * text up to lastcol.  If lastcol < cursor, no text replaced.
 */
{
	register struct sc_screen *sc;
	register struct li_line   *line;
	boolean	quick_append;
	INT	i;
	char	*s, *indentbuf;
	void	fake_chars();

	file.fi_modified = TRUE;
	/*
	 * sc_firstcol = first column of insertion. Fixed.
	 * sc_lastcol  = last column of insertion.  Grows.
	 * sc_column   = current cursor position
	 */

nextinsert:
	input_mode = TRUE;
	quick_append = FALSE;
	indentbuf = NULL;
	save_Undo();
	sc = &screen;
	line = sc->sc_curline;

	if (!opened_line) {
		/*
		 * Remember insertion for later undo
		 */
		undo.un_firstline = sc->sc_firstline;
		undo.un_lastline = sc->sc_lastline;
		undo.un_validcol = TRUE;
		undo.un_firstcol = sc->sc_firstcol;
		undo.un_inserted = TRUE;
		/*
		 * Save overwritten text for later undo
		 */
		yank_cmd = ' ';
		if (sc->sc_lastcol >= sc->sc_column) {
			yank();
			undo.un_deleted = TRUE;
		}
	} else if (autoindent > 0) {
		/*
		 * Fake autoindented tabs and spaces into line
		 */
		fake_input = fake_buf;
		indentbuf = xalloc(autoindent+1);
		i = autoindent / set_tabstops;
		if (set_fortran)
			fake_chars(' ', i * set_tabstops);
		else
			fake_chars('\t', i);
		i = autoindent % set_tabstops;
		fake_chars(' ', i);
		fake_input = fake_buf;
		strcpy(indentbuf, fake_input);
	}

	if (sc->sc_lastcol >= sc->sc_column)
		/*
		 * Append '$' to end of replaced text
		 */
		line->li_text[sc->sc_lastcol] = '$';

	line->li_text = realloc(line->li_text, line->li_width + ALLOC_LEN + 1);

	/*
	 * Get and process input chars
	 */
	for (;;) {
		INT	ch;

		/*
		 * Redisplay line
		 * Append is a fast special case
		 */
		if (quick_append)
			quick_append = FALSE;
		else {
			redraw_curline(line->li_text);
			move_cursor(sc->sc_lineno, sc->sc_column);
		}

		ch = rv_getchar();
		if (superquote) /* If quoted with ^V, skip all processing */
			goto addchar;

		if (ch == erasechar() || ch == '\b') {
			/*
			 * Delete a character if not blackslashed
			 */
			if (sc->sc_column == sc->sc_firstcol) {
				flash();
				continue;
			}
			if (line->li_text[sc->sc_column-1] == '\\')
				line->li_text[sc->sc_column-1] = ch;
			else
				sc->sc_column--;
			continue;
		}

		if (ch == killchar() || ch == CTRL(X)) {
			/*
			 * Delete line if not backslashed
			 */
			if (sc->sc_column == sc->sc_firstcol) {
				/*
				 * Already at first column, error
				 */
				flash();
				continue;
			}
			if (line->li_text[sc->sc_column-1] == '\\')
				line->li_text[sc->sc_column-1] = ch;
			else
				sc->sc_column = sc->sc_firstcol;
			continue;
		}

		switch (ch) {
case CTRL([):
case '\n':
case '\r':
		/*
		 * Escape or newline - end insert
		 */
		if (replace_flag) {
			/*
			 * Clean up replace mode
			 */
			if (sc->sc_column <= sc->sc_lastcol) {
				register i;
				/*
				 * Replace backspaced chars from undo buffer
				 */
				for (i=sc->sc_column; i <= sc->sc_lastcol; ++i)
					if (i < sc->sc_origline.li_width)
						line->li_text[i] =
						    sc->sc_origline.li_text[i];
			}
			/*
			 * Chop off backspaced chars at end of line
			 */
			if (sc->sc_lastcol == line->li_width-1 &&
					sc->sc_lastcol >= 0) {
				line->li_width = sc->sc_column;
				line->li_text[line->li_width] = '\0';
			}
		} else { /* insert mode */
			if (sc->sc_lastcol >= sc->sc_column) {
				register char	*s, *s2;
				/*
				 * Close up backspaced text
				 */
				s  = &line->li_text[sc->sc_column];
				s2 = &line->li_text[sc->sc_lastcol+1];
				while (*s++ = *s2++)
					;
				line->li_width = strlen(line->li_text);
			}
		}

		/*
		 * Collapse autoindent if line not altered by user
		 */
		if (opened_line && autoindent > 0 && indentbuf &&
				strcmp(indentbuf, line->li_text) == 0) {
			redraw_curline("");
			sc->sc_column = 0;
			i = autoindent;
		} else {
			redraw_curline(line->li_text);
			i = 0;
		}
		if (indentbuf)
			free(indentbuf);

		if (ch == CTRL([)) {
			input_mode = FALSE;
			replace_flag = FALSE;
			if (sc->sc_column > 0)
				sc->sc_column--;
			move_cursor(sc->sc_lineno, sc->sc_column);
			if (sc->sc_lastcol < sc->sc_firstcol) {
				if (!opened_line)
					undo.un_inserted = FALSE;
			} else
				undo.un_lastcol = sc->sc_column;
			return;
		} else {
			register char *s, *s2;
			/*
			 * Continue input on new line
			 */
			s = s2 = &line->li_text[sc->sc_column];
			toss_undo();
			openline(1);
			if (i != 0)
				autoindent = i;
			if (*s != '\0') { /* if not at end of line */
				/*
				 * Split line
				 */
				undo.un_validcol = TRUE;
				undo.un_firstcol = 0;
				if (set_autoindent)
					while (isspace(*s))
						++s;
				redraw_curline(s);
				xmit_curline();
				*s2 = '\0';
				sc->sc_lineno--;
				sc->sc_curline--;
				redraw_curline(sc->sc_curline->li_text);
				save_Undo();
				xmit_curline();
				move_cursor(sc->sc_lineno+1, 0);
				save_Undo();
			}
			goto nextinsert; /* quick recursion */
		}
		break;

case CTRL(W):
		/*
		 * Control-W - backspace 1 word
		 */
		/* Implementation deferred */
		continue;

case '\t':
		/*
		 * Tab
		 */
		if (set_fortran) {
			/*
			 * Fortran source.  Expand tab to spaces
			 */
			if (sc->sc_column < 6) {
				/*
				 * First tab in fortran program, expand
				 * to six spaces
				 */
				fake_input = fake_buf;
				fake_chars(' ', 6-sc->sc_column);
				fake_input = fake_buf;
				continue;
			}
			/*
			 * Expand other tabs to shiftwidth spaces
			 */
			i = set_shiftwidth - (sc->sc_column % set_shiftwidth);
			fake_input = fake_buf;
			fake_chars(' ', i);
			fake_input = fake_buf;
			continue;
		}
		break;


case CTRL(D):
		/*
		 * Backtab
		 */
		if (sc->sc_column == sc->sc_firstcol) {
			flash();
			continue;
		}
		i = screen_column(line->li_text, sc->sc_column);
		i -= i % set_shiftwidth == 0 ?
			set_shiftwidth : i % set_shiftwidth;
		i = file_column(line->li_text, i);
		if (set_fortran && i < 6 && sc->sc_column > 6)
			i = 6;
		fake_input = fake_buf;
		fake_chars('\b', sc->sc_column - i);
		fake_input = fake_buf;
		continue;

		} /* End switch */

		

		/*
		 * Add character to line
		 */

addchar:
		/*
		 * Check if line needs expansion
		 */
		if (sc->sc_lastcol < sc->sc_column) {
			if (!replace_flag || sc->sc_column >= line->li_width) {
				register  char *s, *s2;
				/*
				 * Expand line 1 character
				 */
				line->li_width++;
				/*
				 * Expand line in chunks of ALLOC_LEN
				 */
				if (line->li_width % ALLOC_LEN == 0)
					line->li_text = realloc(line->li_text,
						line->li_width + ALLOC_LEN+1);

				if (sc->sc_column == line->li_width-1 && 
				    ch >= '@' && CURCOLUMN > 0) {
					/*
					 * Fast special case - add char at
					 * eol.  No control chars, no char, 
					 * shifting, and no line shifting
					 * required.
					 */
					quick_append = TRUE;
					addch(ch);
					line->li_text[line->li_width] = '\0';
#ifndef USG
					/*
					 * KLUDGE - Handle VT100 brain damage
					 * until Berkeley fixes up curses to 
					 * handle it.
					 */
					if (XN && CURCOLUMN==0) {
						addch(' ');
						refresh();
						addch('\b');
					}
#endif
				} else {
					/*
					 * Slide text beyond insert right
					 */
					s  = &line->li_text[line->li_width-1];
					s2 = &line->li_text[sc->sc_column];
					while (s >= s2) {
						*(s+1) = *s;
						--s;
					}
				}
			}
			sc->sc_lastcol++;
			if (sc->sc_lastcol < sc->sc_firstcol)
				sc->sc_lastcol = sc->sc_firstcol;
		}

		/*
		 * Insert character
		 */
		line->li_text[sc->sc_column++] = ch;
	}
}


INT
rv_getchar()
/*
 * Get a character from the keyboard.  
 * Parse ^V as a super quote.
 */
{
	INT ch;

	superquote = FALSE;
	/*
	 * Check for fake input
	 */
	if (fake_input && *fake_input != '\0')
		return(*fake_input++);

	refresh();
	while ((ch = getch()) == '\0') /* null is verboten */
		;
	if (ch == CTRL(V)) {
		raw(); /* Allow ^S, ^Q and interrupt keys */
		insch('^');
		/*
		 * Get raw character, null is verboten.
		 */
		refresh();
		while ((ch = getch()) == '\0')
			;
		noraw();
#ifdef USG
		cbreak();
#else
		crmode();
#endif !USG
		superquote = (ch != '\n');
	}
	if (ch == ERR || ch == EOF)
		quit();
	return(ch);
}


void
dup_insert(count)
/*
 * Duplicate the text found between firstcol and column
 */
INT count;
{
	register struct sc_screen *sc;
	register struct li_line   *line;
	register char	*buf, *s, *s1, *s2;

	sc = &screen;
	line = sc->sc_curline;

	/*
	 * Make sure duplicatable section exists
	 */
	if (count <= 1 || sc->sc_column  < sc->sc_firstcol
		       || sc->sc_lastcol < sc->sc_firstcol)
		return;

	sc->sc_column++;
	/*
	 * Get buffer space
	 */
	buf = xalloc(strlen(line->li_text)+1 + (sc->sc_column -
					     sc->sc_firstcol) * count);
	/*
	 * Copy text up to duplicatable section
	 */
	s = buf;
	s1 = line->li_text;
	s2 = &line->li_text[sc->sc_column];
	while (s1 < s2)
		*s++ = *s1++;

	/*
	 * Duplicate section
	 */
	while (--count > 0) {
		s1 = &line->li_text[sc->sc_firstcol];
		while (s1 < s2)
			*s++ = *s1++;
	}

	/*
	 * Position cursor at last character of last duplicated section
	 */
	sc->sc_column = (s - buf) - 1;

	/*
	 * Set up for undo
	 */
	if (!opened_line) {
		undo.un_inserted = TRUE;
		undo.un_validcol = TRUE;
		undo.un_firstline = undo.un_lastline = sc->sc_lineno;
		undo.un_firstcol = sc->sc_firstcol;
		undo.un_lastcol = sc->sc_column;
	}

	/*
	 * Copy remainder of line
	 */
	while (*s++ = *s2++)
		;
	redraw_curline(buf);
	free(buf);
	move_cursor(sc->sc_lineno, sc->sc_column);
}


void
fake_chars(ch, count)
char ch;
register INT count;
{
	register char *s;

	s = fake_input;
	for (; count > 0; --count)
		*s++ = ch;
	*s = '\0';
	fake_input = s;
}
