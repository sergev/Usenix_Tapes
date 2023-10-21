#

/*LINTLIBRARY*/

#ifndef lint
#ifndef NOID
static char	sccsid[] = "@(#)ctime.c	3.1";
#endif /* !NOID */
#endif /* !lint */

#ifndef BSD_COMPAT

/*
** On non-BSD systems, this can be a separate function (as is proper).
*/

#include "sys/types.h"
#include "time.h"

extern char *		asctime();
extern struct tm *	localtime();

char *
ctime(timep)
time_t *	timep;
{
	return asctime(localtime(timep));
}

#endif /* !BSD_COMPAT */
