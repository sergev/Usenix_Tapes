/*
** vn news reader.
**
** tmpnam.c - tmpnam() replacement for UCB, also uses non-generic name.
**
** see copyright disclaimer / history in vn.c source file
*/

#include <stdio.h>
#include "config.h"

char *tmpnam (buf)
char *buf;
{
	static char *ptr = VNTEMPNAME;

	/* depends on string initialized above */
	sprintf (ptr+TMP_XOFFSET,"XXXXXX");

	mktemp (ptr);

	if (buf != NULL)
		strcpy (buf,ptr);

	return (ptr);
}
