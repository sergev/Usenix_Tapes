#include "rv.h"
#include "regexp.h"

#ifdef CRAY
#define move_cursor_flag   mc_flag
#endif

static char prev_search[256];   /* Buffer holding previous search string */
static char buf[256];		/* Buffer holding current search string */
static regexp *cmp;		/* Compiled regular expression (RE) */

boolean
search(direction, txt, move_cursor_flag)
/*
 * Search file for match
 *
 * Scan current line using regexec() for txt
 * If not found, xmit search request to ed
 *      Scan line that ed finds using regex()
 * Put cursor at new location, if move_cursor_flag
 */
INT  direction;   /* Direction of search (-1, 1) */
char *txt;        /* Regular expression to search for */
boolean move_cursor_flag; /* TRUE if cursor to be moved to match */
{
	register struct sc_screen *sc;
	register char *s, *s2;
	register INT i, ch;
	struct li_line   *line;
	INT offset = 0;
	char *found, numbuf[20];

	sc = &screen;
	sc->sc_validcol = TRUE;
	sc->sc_firstcol = sc->sc_lastcol = sc->sc_column;
	sc->sc_firstline = sc->sc_lastline = sc->sc_lineno;

	if (txt == NULL)
		return FALSE;
	strcpy(buf, txt);
	if (direction >= 0)
		ch = '/';
	else
		ch = '?';
	s = buf;
	/*
	 * Scan for first / or ? not preceeded by an odd number of backslashes
	 */
	i = 0;
	while (*s != '\0' && (*s != ch || i & 1)) {
		i = *s == '\\' ? i+1 : 0;
		++s;
	}
	/*
	 * Drop '\0' into matching / or ?
	 */
	if (*s != '\0')
		*s++ = '\0';
	ch = *s;

	/*
	 * Check for +/- offset
	 */
	if (ch == '+' || ch == '-') {
		offset = atoi(s);
		if (offset == 0 && *(s+1) != '0')
			offset = 1;
		if (*s == '-')
			offset = -offset;
		sc->sc_validcol = FALSE;
	}

	if (*buf == '\0')
		strcpy(buf, prev_search);
	if (*buf == '\0') {
		botprint(TRUE, "No remembered search string");
		return FALSE;
	}
	/*
	 * Compile RE
	 */
	if (cmp)
		free(cmp);
	if ((cmp = regcomp(buf)) == 0) {
		return FALSE;
	}

	strcpy(prev_search, buf);

	line = sc->sc_curline;
	s = &line->li_text[sc->sc_column];
	found = NULL;
	if (direction >= 0) {
		/*
		 * Search current line after cursor 
		 */
		if (*buf != '^' && *++s != '\0')
			if (regexec(cmp, s) != 0) {
				found = cmp->startp[0];
				sc->sc_lastcol = found - line->li_text - 1;
			}
	} else {
		/*
		 * Search current line before cursor
		 */
		s2 = line->li_text;
		while (s2 < s) {
			if (regexec(cmp, s2) == 0 || cmp->startp[0] >= s)
				break;
			found = cmp->startp[0];
			if (*buf == '^') /* If anchored */
				break;
			s2 = cmp->startp[0]+1;
		}
		if (found) {
			sc->sc_lastcol--;
			sc->sc_firstcol = found - line->li_text;
		}
	}

	if (found) {
		/*
		 * Found match on current line
		 */
		 if (offset != 0) {
			/*
			 * Line offset requested from current line (sigh)
			 */
			sc->sc_validcol = FALSE;
			if ((i = sc->sc_lineno + offset) < 1) {
				botprint(TRUE, "Negative address");
				return FALSE;
			}
			if (i > file.fi_numlines) {
				botprint(TRUE, "Not that many lines in buffer");
				return FALSE;
			}
			if (i < sc->sc_lineno)
				sc->sc_firstline = i;
			else if (i > sc->sc_lineno)
				sc->sc_lastline = i-1;
			if (move_cursor_flag)
				move_abs_cursor(i, COL_FIRST_NONWHITE);
			return TRUE;
		}
		if (move_cursor_flag)
			if (sc->sc_firstcol != sc->sc_column)
				move_cursor(sc->sc_lineno, sc->sc_firstcol);
			else
				move_cursor(sc->sc_lineno, sc->sc_lastcol+1);
		return TRUE;
        }

	sc->sc_validcol = FALSE;
	/*
	 * Xmit search to ed
	 */
	xmit_curline();
	i = sc->sc_lineno;
	if (direction >= 0)
		xmit_ed("%d\n/%s/\n", i, buf);
	else
		xmit_ed("%d\n?%s?\n", i, buf);
	xmit_sync();
	xmit_ed(".=\n");
	(void) recv_sync(FALSE);
	(void) fgets(numbuf, 18, file.fi_fpin);
	if ((i = atoi(numbuf)) == sc->sc_lineno) {
		botprint(TRUE, "Pattern not found");
		return FALSE;
	}
	if (set_wrapscan == FALSE && 
	    ((direction > 0 && i < sc->sc_lineno) ||
	     (direction < 0 && i > sc->sc_lineno))) { /* If wrapped */
		botprint(TRUE, "Address search hit %s without matching pattern",
			direction < 0 ? "TOP" : "BOTTOM");
		return FALSE;
	}
		
	i += offset;

	if (i < 1) {
		botprint(TRUE, "Negative address");
		return FALSE;
	}
	if (i > file.fi_numlines) {
		botprint(TRUE, "Not that many lines in buffer");
		return FALSE;
	}

	if (i < sc->sc_lineno)
		sc->sc_firstline = i;
	else if (i > sc->sc_lineno)
		sc->sc_lastline = i-1;
	if (!move_cursor_flag)
		return TRUE;
		
	/*
	 * Cursor movement requested
	 */

	/*
	 * Move to line,col of match
	 */
	if (offset != 0) {
		move_abs_cursor(i, COL_FIRST_NONWHITE);
		return TRUE;
	}
	move_abs_cursor(i, 0);
	line = sc->sc_curline;
	if (direction >= 0) {
		if (regexec(cmp, line->li_text) != 0)
			sc->sc_column = cmp->startp[0] - line->li_text;
	} else {
		s2 = line->li_text;
		found = NULL;
		while (*s2 != '\0') {
			if (regexec(cmp, s2) == 0)
				break;
			found = cmp->startp[0];
			if (*buf == '^') /* If anchored */
				break;
			s2 = cmp->startp[0]+1;
		}
		if (found)
			sc->sc_column = found - line->li_text;
	}
	move_cursor(sc->sc_lineno, sc->sc_column);
	return TRUE;
}
