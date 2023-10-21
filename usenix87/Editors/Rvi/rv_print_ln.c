#include "rv.h"

void
print_line(s)
/*
 * Print line to screen at current cursor loc.
 */
char	*s;
{
	char	*uchr;

	--s;
	while(*++s) {
		if (*s < ' ' || *s > '~') /* If control char */
			if (*s == '\t' && !set_list) /* tab */
/* Alan - Expand this using set_tabstops!! */
/* Ugh.. later.  This loop is already too slow.  AEK */
				addch(*s); 
			else {
				/*
				 * Expand control char to ^x
				 */
				uchr = unctrl(*s);
				addch(uchr[0]);
				addch(uchr[1]);
			}
		else
			addch(*s);
	}
	if (set_list)
		addch('$');
#ifndef USG
	/*
	 * KLUDGE - Handle xenl brain damage in the VT100 and
	 * Concept 100 until Berkeley cleans up it's Curses implementation
	 */
	if (XN && CURCOLUMN==0) {
		addch(' '); 
		refresh();
		addch('\b');
	}
#endif
}
