/*
**	Routine for beep for sys V release 2 Xen*x
*/

#include <curses.h>

#define BEEP '\007'

beep()
{
	fputc(BEEP,stdout);
}
