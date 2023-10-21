/*
**	Returns an int corresponding to the delete character
*/
# include <termio.h>

#define DELETE ('\010')

int erasechar()
{
int x;

	return(DELETE);
}

