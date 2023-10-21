/*
 * wgraph(str)
 *
 * translate a string to a graphics rendition, via the "graph" database.
 * The terminal-independent codes in str are:
 *
 *	-	horizontal line
 *	|	vertical line
 *	+	cross
 *	T	upper tee
 *	i	lower tee (look at an `i' on a vt100 for details)
 *	>	left tee
 *	<	right tee
 *	,	LR corner
 *	.	UR corner (backslash would be preferable but you can't gage it)
 *	`	LL corner
 *	/	UL corner
 *
 * On SVR2, this provides SVR3 curses characters ACS_* for use everywhere.
 */

#include <curses.h>
#ifndef ACS_DIAMOND
#include "tgraph.h"
#define _TGLOAD_
#endif  ACS_DIAMOND

wgraph(win, str)
WINDOW *win;
char *str; {
#ifdef _TGLOAD_
	static int tgr = 0;
	
	if (tgr == 0) {
		__tgr_load();
		tgr = 1;
	}
#endif _TGLOAD_
	while (*str != '\0') {
		switch (*str) {
		case '\n':
			waddch(win, '\n');
			break;
		case '-':
			waddch(win, ACS_HLINE);
			break;
		case '|':
			waddch(win, ACS_VLINE);
			break;
		case '+':
			waddch(win, ACS_PLUS);
			break;
		case '>':
			waddch(win, ACS_LTEE);
			break;
		case '<':
			waddch(win, ACS_RTEE);
			break;
		case 'T':
			waddch(win, ACS_TTEE);
			break;
		case 'i':
			waddch(win, ACS_BTEE);
			break;
		case '`':
			waddch(win, ACS_LLCORNER);
			break;
		case '/':
			waddch(win, ACS_ULCORNER);
			break;
		case ',':
			waddch(win, ACS_URCORNER);
			break;
		case '.':
			waddch(win, ACS_LRCORNER);
			break;
		case '_':
			waddch(win, ACS_S9);
			break;
		case '=':
			waddch(win, ACS_S1);
			break;
		case '^':
			waddch(win, ACS_UARROW);
			break;
		case 'v':
			waddch(win, ACS_DARROW);
			break;
		case '(':
			waddch(win, ACS_LARROW);
			break;
		case ')':
			waddch(win, ACS_RARROW);
			break;
		case '$':
			waddch(win, ACS_DIAMOND);
			break;
		case '#':
			waddch(win, ACS_CKBOARD);
			break;
		case 'o':
			waddch(win, ACS_DEGREE);
			break;
		case '~':
			waddch(win, ACS_PLMINUS);
			break;
		case '*':
			waddch(win, ACS_BULLET);
			break;
		case 'X':
			waddch(win, ACS_BOARD);
			break;
		case 'I':
			waddch(win, ACS_LANTERN);
			break;
		case '@':
			waddch(win, ACS_BLOCK);
			break;
		default:
			waddch(win, ' ');
			break;
		}
		str++;
	}
}
