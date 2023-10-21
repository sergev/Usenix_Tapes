#include "rv.h"

void
rv_mark()
{
	register INT	c;

	c = getch();
	if (c < 'a' || c > 'z') {
		botprint(TRUE, "Mark must be a letter from 'a' to 'z'");
		errflag = 1;
		return;
	}
	xmit_curline();
	xmit_ed("%dk%c\n", screen.sc_lineno, c);
}
