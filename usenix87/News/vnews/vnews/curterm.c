/*
 * Additions to curses.
 */

#include <stdio.h>
#include <curses.h>

/*
 * move to the bottom of the screen.
 */

botscreen() {
	move(LINES-1, 0);
	refresh();
	putchar('\n');
	fflush(stdout);
}


/*
 * Clear a line.
 */

clrline(linno) {
	move(linno, 0);
	clrtoeol();
}


#ifdef ACURSES
/*
 * Ring the bell on the terminal.
 */

beep() {
	putchar('\007');
	fflush(stdout);
}
#endif
