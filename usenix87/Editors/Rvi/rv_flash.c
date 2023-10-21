#include "rv.h"

/*
 * Simulate flash() on non-USG curses
 */

#ifndef USG
extern int _putchar();
void
flash()
{
	static INT once;
	static char flashstr[80];
	char *tgetstr(), *s;

	if (!once) {
		++once;
		s = flashstr;
		if (tgetstr("vb", &s) == NULL) {
			s = flashstr;
			if (tgetstr("bl", &s) == NULL)
				strcpy(flashstr, "\007");
		}
	}

	_puts(flashstr);
}
#endif !USG
