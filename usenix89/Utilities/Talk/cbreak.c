/*
**	cbreak (a la system V R2)
*/

#include <curses.h>

cbreak()
{
	crmode();
}
