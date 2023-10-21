#

/*LINTLIBRARY*/

#ifndef lint
#ifndef NOID
static char	sccsid[] = "@(#)dysize.c	3.1";
#endif /* !NOID */
#endif /* !lint */

#ifdef BSD_COMPAT

#include "tzfile.h"

dysize(y)
{
	/*
	** The 4.[0123]BSD version of dysize behaves as if the return statement
	** below read
	**	return ((y % 4) == 0) ? DAYS_PER_LYEAR : DAYS_PER_NYEAR;
	** but since we'd rather be right than (strictly) compatible. . .
	*/
	return isleap(y) ? DAYS_PER_LYEAR : DAYS_PER_NYEAR;
}

#endif /* BSD_COMPAT */
