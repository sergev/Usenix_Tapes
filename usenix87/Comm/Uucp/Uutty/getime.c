#include "uutty.h"
/* 
** Get the current local time, return pointer to the ASCII version.
*/
char *getime()
{
		time(&currtime);		/* 32-bit UNIX timestamp */
		ctimep = ctime(&currtime);	/* Convert to ASCII */
		ctimep[24] = '\0';		/* We don't want the newline */
		return ctimep;
}
