#include	"godef.h"
#include	<curses.h>
/*
**      BDISP -- board graphics for gref, GO referee program
**	CURSINIT -- One-time initialization for bdisp()
**	CURSFINI -- One-time cleanup
**	BDREDRAW -- Re-draw the screen if messed up
**	DISLOG -- display a log entry
**      psl 6/84
*/

#define	SCRNH		24		/* assume 24-lines for the moment */

extern	char	*letters;		/* from stoc.c */

extern	char    *copy();

cursinit()
{
	register char *cp;
	register int i, j;

	initscr();
	leaveok(stdscr, TRUE);
	/* top border */
	for (i = 1; i < BSZ1; i++) {
	    move(0, 2 * i + 1);
	    addch(letters[i]);
	}
	/* left and right edges */
	for (j = BSZ1; --j > 0; ) {
	    move(20 - j, 0);
	    printw("%2d ", j);
	    move(20 - j, 2 * BSZ1 + 1);
	    printw("%d ", j);
	}
	/* bottom border */
	for (i = 1; i < BSZ1; i++) {
	    move(20, 2 * i + 1);
	    addch(letters[i]);
	}
	move(0, 47);
	addstr("#  black  white   time");
	cp = copy(p[BLACK].p_plyr, fmtbuf);
	i = 6 - (cp - fmtbuf) / 2;
	move(21, i > 0? i : 0);
	printw("BLACK/%s", p[BLACK].p_plyr);
	cp = copy(p[WHITE].p_plyr, fmtbuf);
	i = 30 - (cp - fmtbuf) / 2;
	move(21, i);
	printw("WHITE/%s", p[WHITE].p_plyr);
	move(21, 19);
	addstr(" vs. ");
}

/*      Update board display
*/
bdisp()
{
	register int i, j;

	move(22, 7);
	printw("%3d       PRISONERS     %3d",
	 p[BLACK].p_captures, p[WHITE].p_captures);
	move(23, 5);
	printw("%2d:%02d:%02d    USER TIME   %3d:%02d:%02d", 
	 (int) (p[BLACK].p_utime / 3600),
	 (int) ((p[BLACK].p_utime % 3600) / 60),
	 (int) (p[BLACK].p_utime % 60),
	 (int) (p[WHITE].p_utime / 3600),
	 (int) ((p[WHITE].p_utime % 3600) / 60),
	 (int) (p[WHITE].p_utime % 60));
	for (j = BSZ1; --j > 0; ) {
	    for (i = 1; i < BSZ1; i++) {
		move(BSZ1 - j, 2 * i + 1);
		addch(".*O?"[b.b_spot[i + j * BSZ2].s_occ]);
	    }
	}
	refresh();
}

cursfini()
{
	leaveok(stdscr, FALSE);
	move(22, 70);
	refresh();
	endwin();
}

bdredraw()
{
	wrefresh(curscr);
}

update()
{
	refresh();
}

/*      Display a transcript entry
*/
dislog(str)
char	*str;
{
	static int lastline;

	if (++lastline >= SCRNH - 1) {
	    /* move 'em up */
	    lastline = 1;
	}
	move(lastline, 46);
	addstr(str);
	move(lastline + 1, 46);
	clrtoeol();
}
