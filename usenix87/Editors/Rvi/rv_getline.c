#include "rv.h"
#include <ctype.h>

extern boolean did_botprint;  /* Used by hitcr_continue() */
extern boolean scrolled;      /* Used by hitcr_continue() */
extern boolean superquote;    /* Set by rv_getchar() */

static char buf[256];

char *
rv_getline()
/*
 * Read input line
 * Returns a static buffer, else NULL
 */
{
	char *s, ch;
	INT savecol, saverow;

	s = buf;
	strncpy(buf, "", 256);  /* Zero out buf */
	savecol = CURCOLUMN;
	saverow = CURLINE;
	hitcr_continue();
	clrtobot();
	move(saverow, savecol);  /* Because clrtobot homes the cursor in 4bsd */
	did_botprint = TRUE;

	for (;;) {
		ch = rv_getchar();
		if (!superquote) {
			if (ch == erasechar())
				ch = '\b';
			if (ch == killchar())
				ch = CTRL(X);
			switch (ch) {
case '\b':
		/*
		 * Backspace
		 */
		if (s-- <= buf) {
			if (screen_column(buf, strlen(buf)) >= COLS-1)
				scrolled = TRUE;
			hitcr_continue();
			move_cursor(screen.sc_lineno, screen.sc_column);
			return NULL;
		}
		move(CURLINE, CURCOLUMN-1);
		if (*s < ' ' || *s > '~')
			move(CURLINE, CURCOLUMN-1);
		clrtoeol();
		break;

case CTRL(X):
		/*
		 * Line kill
		 */
		s = buf;
		move(saverow, savecol);
		clrtobot();
		move(saverow, savecol);
		break;

case '\t':
		/*
		 * Tab
		 */
		addch('^');
		addch('I');
		*s++ = ch;
		break;

case '\r':
case '\n':
case CTRL([):
		/*
		 * End of line 
		 */
		if (screen_column(buf, strlen(buf)) >= COLS-1)
			scrolled = TRUE;
		*s = '\0';
		hitcr_continue();
		move_cursor(screen.sc_lineno, screen.sc_column);
		return buf;

default:
		addch(ch);
		*s++ = ch;
		break;

			} /* End switch */
		} else {
			addch(ch);
			*s++ = ch;
		}

		if (buf-s >= 250) { /* If end of buffer */
			scrolled = TRUE;
			*s = '\0';
			hitcr_continue();
			move_cursor(screen.sc_lineno, screen.sc_column);
			return buf;
		}
	}
}
