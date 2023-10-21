/*
**	I am not real sure what flushinp was supposed to be doing
**	So I did this, seems to work.
*/

#include <curses.h>

flushinp()
{
	fflush(stdin);
}
